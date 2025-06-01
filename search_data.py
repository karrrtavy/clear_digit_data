import os
import re
import tkinter as tk
from tkinter import ttk, messagebox, scrolledtext, filedialog
import hashlib

class PersonalDataCleanerApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Поиск и очистка личных данных")
        self.root.geometry("800x600")
        
        self.search_path = tk.StringVar()
        self.action = tk.StringVar(value="find")
        
        ttk.Label(root, text="Папка для поиска:").pack(pady=5)
        ttk.Entry(root, textvariable=self.search_path, width=50).pack(pady=5)
        ttk.Button(root, text="Выбрать папку", command=self.select_directory).pack(pady=5)
        
        ttk.Label(root, text="Действие с найденными данными:").pack(pady=5)
        
        ttk.Radiobutton(root, text="Только поиск (не изменять файлы)", 
                       variable=self.action, value="find").pack(anchor="w")
        ttk.Radiobutton(root, text="Заменить на ***", 
                       variable=self.action, value="replace").pack(anchor="w")
        ttk.Radiobutton(root, text="Удалить файл", 
                       variable=self.action, value="delete").pack(anchor="w")
        ttk.Radiobutton(root, text="Зашифровать", 
                       variable=self.action, value="encrypt").pack(anchor="w")
        
        ttk.Button(root, text="Начать поиск", command=self.start_search).pack(pady=10)
        
        self.log_area = scrolledtext.ScrolledText(root, width=90, height=25, state='disabled')
        self.log_area.pack(pady=10)
    
    def select_directory(self):
        folder = filedialog.askdirectory()
        if folder:
            self.search_path.set(folder)
    
    def log(self, message):
        self.log_area.configure(state='normal')
        self.log_area.insert(tk.END, message + "\n")
        self.log_area.configure(state='disabled')
        self.log_area.see(tk.END)
        self.root.update()
    
    def find_personal_data(self, text):
        patterns = {
            "Телефон (РФ)": r"(\+7|8)[\s\-]?\(?\d{3}\)?[\s\-]?\d{3}[\s\-]?\d{2}[\s\-]?\d{2}",
            "Email": r"\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Z|a-z]{2,}\b",
            "Кредитная карта": r"\b\d{4}[\s\-]?\d{4}[\s\-]?\d{4}[\s\-]?\d{4}\b",
            "Паспорт (РФ)": r"\b\d{4}\s?\d{6}\b"
        }
        
        found = {}
        for name, regex in patterns.items():
            matches = re.findall(regex, text)
            if matches:
                found[name] = matches
        return found
    
    def process_file(self, filepath):
        try:
            with open(filepath, 'r', encoding='utf-8') as f:
                content = f.read()
        except UnicodeDecodeError:
            self.log(f"⚠ Файл {os.path.basename(filepath)} — бинарный (пропускаем)")
            return
        except Exception as e:
            self.log(f"⚠ Ошибка чтения {filepath}: {e}")
            return
        
        found_data = self.find_personal_data(content)
        if not found_data:
            return
        
        self.log(f"\n🔍 Найдено в {os.path.basename(filepath)}:")
        for data_type, items in found_data.items():
            self.log(f"  {data_type}: {', '.join(items[:3])}" + ("..." if len(items) > 3 else ""))
        
        action = self.action.get()
        
        if action == "find":
            self.log("  ℹ Режим поиска - файл не изменен")
            return
            
        elif action == "replace":
            new_content = content
            for items in found_data.values():
                for item in items:
                    new_content = new_content.replace(item, "***")
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(new_content)
            self.log("  ✅ Данные заменены на '***'")
        
        elif action == "delete":
            os.remove(filepath)
            self.log("  ✅ Файл удалён")
        
        elif action == "encrypt":
            encrypted = hashlib.sha256(content.encode()).hexdigest()
            with open(filepath + ".encrypted", 'w') as f:
                f.write(encrypted)
            os.remove(filepath)
            self.log("  ✅ Файл зашифрован и удалён")
    
    def start_search(self):
        folder = self.search_path.get()
        if not folder or not os.path.exists(folder):
            messagebox.showerror("Ошибка", "Укажите корректную папку!")
            return
        
        self.log("\n" + "="*50)
        self.log(f"🔎 Поиск в папке: {folder}")
        
        action = self.action.get()
        if action == "find":
            self.log("⚙ Режим: Только поиск (файлы не изменяются)")
        elif action == "replace":
            self.log("⚙ Режим: Замена данных на '***'")
        elif action == "delete":
            self.log("⚙ Режим: Удаление файлов с личными данными")
        else:
            self.log("⚙ Режим: Шифрование и удаление файлов")
        
        supported_ext = ('.txt', '.csv', '.json', '.xml', '.html', '.log', '.md')
        total_files = 0
        found_files = 0
        
        for root_dir, _, files in os.walk(folder):
            for file in files:
                if file.endswith(supported_ext):
                    total_files += 1
                    if self.process_file(os.path.join(root_dir, file)):
                        found_files += 1
        
        self.log("\n" + "="*50)
        self.log(f"🔎 Итоги поиска:")
        self.log(f"• Проверено файлов: {total_files}")
        self.log(f"• Найдено файлов с личными данными: {found_files}")
        self.log(f"• Режим обработки: {'только поиск' if action == 'find' else action}")
        self.log("\nГотово! Проверьте лог.")

if __name__ == "__main__":
    root = tk.Tk()
    app = PersonalDataCleanerApp(root)
    root.mainloop()