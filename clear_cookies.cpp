#include <QApplication>
#include <QMainWindow>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QProcessEnvironment>
#include <QDebug>

class BrowserCleanerApp : public QMainWindow {
    Q_OBJECT

public:
    BrowserCleanerApp(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Очистка данных браузеров");
        setGeometry(100, 100, 700, 500);

        QWidget *centralWidget = new QWidget(this);
        QVBoxLayout *layout = new QVBoxLayout(centralWidget);

        logArea = new QTextEdit(this);
        logArea->setReadOnly(true);
        layout->addWidget(logArea);

        QCheckBox *chromeCheck = new QCheckBox("Google Chrome", this);
        QCheckBox *firefoxCheck = new QCheckBox("Mozilla Firefox", this);
        QCheckBox *edgeCheck = new QCheckBox("Microsoft Edge", this);
        QCheckBox *operaCheck = new QCheckBox("Opera", this);
        QCheckBox *yandexCheck = new QCheckBox("Яндекс.Браузер", this);

        chromeCheck->setChecked(true);
        firefoxCheck->setChecked(true);
        edgeCheck->setChecked(true);
        operaCheck->setChecked(true);
        yandexCheck->setChecked(true);

        layout->addWidget(chromeCheck);
        layout->addWidget(firefoxCheck);
        layout->addWidget(edgeCheck);
        layout->addWidget(operaCheck);
        layout->addWidget(yandexCheck);

        QPushButton *cleanButton = new QPushButton("Очистить данные", this);
        layout->addWidget(cleanButton);

        connect(cleanButton, &QPushButton::clicked, this, [=]() {
            runCleaner(
                chromeCheck->isChecked(),
                firefoxCheck->isChecked(),
                edgeCheck->isChecked(),
                operaCheck->isChecked(),
                yandexCheck->isChecked()
            );
        });

        setCentralWidget(centralWidget);
    }

private:
    QTextEdit *logArea;

    void log(const QString &message) {
        logArea->append(message);
        QApplication::processEvents();
    }

    void clearChromeData(const QString &browserName, const QString &browserPath) {
        QString userData = QProcessEnvironment::systemEnvironment().value("LOCALAPPDATA") + 
                         "/" + browserPath + "/User Data";
        
        if (!QDir(userData).exists()) {
            log(QString("[%1] Не найден, пропускаем...").arg(browserName));
            return;
        }

        log(QString("\n=== Очистка %1 ===").arg(browserName));
        
        QStringList profiles = {"Default"};
        for (int i = 1; i <= 4; ++i) {
            profiles.append(QString("Profile %1").arg(i));
        }

        for (const QString &profile : profiles) {
            QString profilePath = userData + "/" + profile;
            if (!QDir(profilePath).exists()) continue;

            log(QString("\n* Профиль: %1").arg(profile));
            
            QStringList filesToDelete = {
                "History", "Cookies", "Login Data", "Top Sites",
                "Web Data", "Favicons", "Bookmarks"
            };

            for (const QString &file : filesToDelete) {
                QString filePath = profilePath + "/" + file;
                if (QFile::exists(filePath)) {
                    if (QFile::remove(filePath)) {
                        log(QString("Удалён файл: %1").arg(file));
                    } else {
                        log(QString("Ошибка удаления: %1").arg(file));
                    }
                }
            }

            QString cacheDir = profilePath + "/Cache";
            if (QDir(cacheDir).exists()) {
                if (removeDir(cacheDir)) {
                    log("Удалена папка: Cache");
                } else {
                    log("Ошибка удаления папки Cache");
                }
            }
        }
    }

    void clearFirefoxData() {
        QString firefoxPath = QProcessEnvironment::systemEnvironment().value("APPDATA") + 
                            "/Mozilla/Firefox/Profiles";
        
        if (!QDir(firefoxPath).exists()) {
            log("[Firefox] Не найден, пропускаем...");
            return;
        }

        log("\n=== Очистка Firefox ===");
        
        QDir profilesDir(firefoxPath);
        QStringList profiles = profilesDir.entryList(QStringList("*.default*"), QDir::Dirs);

        for (const QString &profile : profiles) {
            QString profilePath = firefoxPath + "/" + profile;
            log(QString("\n* Профиль: %1").arg(profile));

            QStringList filesToDelete = {
                "cookies.sqlite", "places.sqlite", "formhistory.sqlite",
                "webappsstore.sqlite", "favicons.sqlite"
            };

            for (const QString &file : filesToDelete) {
                QString filePath = profilePath + "/" + file;
                if (QFile::exists(filePath)) {
                    if (QFile::remove(filePath)) {
                        log(QString("Удалён файл: %1").arg(file));
                    } else {
                        log(QString("Ошибка удаления: %1").arg(file));
                    }
                }
            }

            QString cacheDir = profilePath + "/cache2";
            if (QDir(cacheDir).exists()) {
                if (removeDir(cacheDir)) {
                    log("Удалена папка: cache2");
                } else {
                    log("Ошибка удаления папки cache2");
                }
            }
        }
    }

    bool removeDir(const QString &path) {
        QDir dir(path);
        if (!dir.exists()) return false;

        bool success = true;
        for (const QFileInfo &info : dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs)) {
            if (info.isDir()) {
                success &= removeDir(info.filePath());
            } else {
                success &= QFile::remove(info.filePath());
            }
        }
        success &= dir.rmdir(path);
        return success;
    }

    void runCleaner(bool chrome, bool firefox, bool edge, bool opera, bool yandex) {
        log("\nНачало очистки...");

        if (chrome) {
            clearChromeData("Google Chrome", "Google/Chrome");
        }
        if (edge) {
            clearChromeData("Microsoft Edge", "Microsoft/Edge");
        }
        if (opera) {
            clearChromeData("Opera", "Opera Software/Opera Stable");
        }
        if (yandex) {
            clearChromeData("Яндекс.Браузер", "Yandex/YandexBrowser");
        }
        if (firefox) {
            clearFirefoxData();
        }

        log("\nГотово! Данные удалены.");
        QMessageBox::information(this, "Готово", "Очистка завершена!");
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    BrowserCleanerApp window;
    window.show();
    return app.exec();
}

#include "main.moc"