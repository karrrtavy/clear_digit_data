#include <QApplication>
#include <QMainWindow>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QRegularExpression>
#include <QCryptographicHash>
#include <QDebug>

class PersonalDataCleanerApp : public QMainWindow {
    Q_OBJECT

public:
    PersonalDataCleanerApp(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Поиск и очистка личных данных");
        resize(800, 600);

        QWidget *centralWidget = new QWidget(this);
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

        QLabel *pathLabel = new QLabel("Папка для поиска:", this);
        pathEdit = new QLineEdit(this);
        QPushButton *browseButton = new QPushButton("Выбрать папку", this);
        
        QHBoxLayout *pathLayout = new QHBoxLayout();
        pathLayout->addWidget(pathEdit);
        pathLayout->addWidget(browseButton);

        QLabel *actionLabel = new QLabel("Действие с найденными данными:", this);
        QRadioButton *findOnlyRadio = new QRadioButton("Только поиск (не изменять файлы)", this);
        QRadioButton *replaceRadio = new QRadioButton("Заменить на ***", this);
        QRadioButton *deleteRadio = new QRadioButton("Удалить файл", this);
        QRadioButton *encryptRadio = new QRadioButton("Зашифровать", this);
        
        findOnlyRadio->setChecked(true);
        
        actionGroup = new QButtonGroup(this);
        actionGroup->addButton(findOnlyRadio, 0);
        actionGroup->addButton(replaceRadio, 1);
        actionGroup->addButton(deleteRadio, 2);
        actionGroup->addButton(encryptRadio, 3);

        QPushButton *startButton = new QPushButton("Начать поиск", this);

        logArea = new QTextEdit(this);
        logArea->setReadOnly(true);

        // Компоновка
        mainLayout->addWidget(pathLabel);
        mainLayout->addLayout(pathLayout);
        mainLayout->addWidget(actionLabel);
        mainLayout->addWidget(findOnlyRadio);
        mainLayout->addWidget(replaceRadio);
        mainLayout->addWidget(deleteRadio);
        mainLayout->addWidget(encryptRadio);
        mainLayout->addWidget(startButton);
        mainLayout->addWidget(logArea);

        setCentralWidget(centralWidget);

        connect(browseButton, &QPushButton::clicked, this, &PersonalDataCleanerApp::selectDirectory);
        connect(startButton, &QPushButton::clicked, this, &PersonalDataCleanerApp::startSearch);
    }

private:
    QLineEdit *pathEdit;
    QButtonGroup *actionGroup;
    QTextEdit *logArea;

    void selectDirectory() {
        QString dir = QFileDialog::getExistingDirectory(this, "Выберите папку для поиска");
        if (!dir.isEmpty()) {
            pathEdit->setText(dir);
        }
    }

    void log(const QString &message) {
        logArea->append(message);
        QApplication::processEvents();
    }

    QMap<QString, QStringList> findPersonalData(const QString &content) {
        QMap<QString, QStringList> patterns = {
            {"Телефон (РФ)", R"((\+7|8)[\s\-]?\(?\d{3}\)?[\s\-]?\d{3}[\s\-]?\d{2}[\s\-]?\d{2})"},
            {"Email", R"(\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Z|a-z]{2,}\b)"},
            {"Кредитная карта", R"(\b\d{4}[\s\-]?\d{4}[\s\-]?\d{4}[\s\-]?\d{4}\b)"},
            {"Паспорт (РФ)", R"(\b\d{4}\s?\d{6}\b)"}
        };

        QMap<QString, QStringList> found;
        
        for (auto it = patterns.begin(); it != patterns.end(); ++it) {
            QRegularExpression re(it.value());
            QRegularExpressionMatchIterator matchIter = re.globalMatch(content);
            
            QStringList matches;
            while (matchIter.hasNext()) {
                matches << matchIter.next().captured();
            }
            
            if (!matches.isEmpty()) {
                found[it.key()] = matches;
            }
        }
        
        return found;
    }

    bool processFile(const QString &filePath) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            log(QString("⚠ Ошибка чтения %1: %2").arg(QFileInfo(filePath).fileName(), file.errorString()));
            return false;
        }
        
        QTextStream in(&file);
        in.setCodec("UTF-8");
        QString content = in.readAll();
        file.close();
        
        QMap<QString, QStringList> foundData = findPersonalData(content);
        if (foundData.isEmpty()) {
            return false;
        }
        
        log(QString("\n🔍 Найдено в %1:").arg(QFileInfo(filePath).fileName()));
        for (auto it = foundData.begin(); it != foundData.end(); ++it) {
            QStringList items = it.value();
            QString displayed = items.mid(0, 3).join(", ");
            if (items.size() > 3) displayed += "...";
            log(QString("  %1: %2").arg(it.key(), displayed));
        }
        
        int action = actionGroup->checkedId();
        
        if (action == 0) { 
            log("  ℹ Режим поиска - файл не изменен");
            return true;
        }
        else if (action == 1) { 
            for (const QStringList &items : foundData) {
                for (const QString &item : items) {
                    content.replace(item, "***");
                }
            }
            
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out.setCodec("UTF-8");
                out << content;
                file.close();
                log("  ✅ Данные заменены на '***'");
            } else {
                log(QString("  ⚠ Ошибка записи в файл: %1").arg(file.errorString()));
            }
        }
        else if (action == 2) { 
            if (file.remove()) {
                log("  ✅ Файл удалён");
            } else {
                log(QString("  ⚠ Ошибка удаления файла: %1").arg(file.errorString()));
            }
        }
        else if (action == 3) { 
            QCryptographicHash hash(QCryptographicHash::Sha256);
            hash.addData(content.toUtf8());
            QString encrypted = hash.result().toHex();
            
            QFile encryptedFile(filePath + ".encrypted");
            if (encryptedFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&encryptedFile);
                out << encrypted;
                encryptedFile.close();
                
                if (file.remove()) {
                    log("  ✅ Файл зашифрован и удалён");
                } else {
                    log(QString("  ⚠ Ошибка удаления файла: %1").arg(file.errorString()));
                }
            } else {
                log(QString("  ⚠ Ошибка создания зашифрованного файла: %1").arg(encryptedFile.errorString()));
            }
        }
        
        return true;
    }

    void startSearch() {
        QString folder = pathEdit->text();
        if (folder.isEmpty() || !QDir(folder).exists()) {
            QMessageBox::critical(this, "Ошибка", "Укажите корректную папку!");
            return;
        }
        
        log("\n" + QString("=" * 50));
        log(QString("🔎 Поиск в папке: %1").arg(folder));
        
        int action = actionGroup->checkedId();
        QString actionStr;
        
        switch (action) {
            case 0: actionStr = "Только поиск (файлы не изменяются)"; break;
            case 1: actionStr = "Замена данных на '***'"; break;
            case 2: actionStr = "Удаление файлов с личными данными"; break;
            case 3: actionStr = "Шифрование и удаление файлов"; break;
        }
        
        log(QString("⚙ Режим: %1").arg(actionStr));
        
        QStringList supportedExt = {".txt", ".csv", ".json", ".xml", ".html", ".log", ".md"};
        int totalFiles = 0;
        int foundFiles = 0;
        
        QDirIterator it(folder, supportedExt, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString filePath = it.next();
            totalFiles++;
            
            if (processFile(filePath)) {
                foundFiles++;
            }
        }
        
        log("\n" + QString("=" * 50));
        log("🔎 Итоги поиска:");
        log(QString("• Проверено файлов: %1").arg(totalFiles));
        log(QString("• Найдено файлов с личными данными: %1").arg(foundFiles));
        log(QString("• Режим обработки: %1").arg(actionStr));
        log("\nГотово! Проверьте лог.");
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    
    PersonalDataCleanerApp window;
    window.show();
    
    return app.exec();
}

#include "main.moc"