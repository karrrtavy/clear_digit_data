import os
import shutil
import sqlite3
import glob
import tkinter as tk
from tkinter import ttk, messagebox, scrolledtext

class BrowserCleanerApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Очистка данных браузеров")
        self.root.geometry("700x500")
        
        self.chrome_var = tk.BooleanVar(value=True)
        self.firefox_var = tk.BooleanVar(value=True)
        self.edge_var = tk.BooleanVar(value=True)
        self.opera_var = tk.BooleanVar(value=True)
        self.yandex_var = tk.BooleanVar(value=True)
        
        self.log_area = scrolledtext.ScrolledText(root, width=80, height=20, state='disabled')
        self.log_area.pack(pady=10)
        
        settings_frame = ttk.LabelFrame(root, text="Выберите браузеры для очистки")
        settings_frame.pack(pady=5, padx=10, fill="x")
        
        ttk.Checkbutton(settings_frame, text="Google Chrome", variable=self.chrome_var).pack(anchor="w")
        ttk.Checkbutton(settings_frame, text="Mozilla Firefox", variable=self.firefox_var).pack(anchor="w")
        ttk.Checkbutton(settings_frame, text="Microsoft Edge", variable=self.edge_var).pack(anchor="w")
        ttk.Checkbutton(settings_frame, text="Opera", variable=self.opera_var).pack(anchor="w")
        ttk.Checkbutton(settings_frame, text="Яндекс.Браузер", variable=self.yandex_var).pack(anchor="w")
        
        ttk.Button(root, text="Очистить данные", command=self.run_cleaner).pack(pady=10)
    
    def log(self, message):
        self.log_area.configure(state='normal')
        self.log_area.insert(tk.END, message + "\n")
        self.log_area.configure(state='disabled')
        self.log_area.see(tk.END)
        self.root.update()
    
    def clear_chrome_data(self, browser_name, browser_path):
        user_data = os.path.join(os.getenv("LOCALAPPDATA"), browser_path, "User Data")
        if not os.path.exists(user_data):
            self.log(f"[{browser_name}] Не найден, пропускаем...")
            return
        
        self.log(f"\n=== Очистка {browser_name} ===")
        profiles = ["Default"] + [f"Profile {i}" for i in range(1, 5)]
        
        for profile in profiles:
            profile_path = os.path.join(user_data, profile)
            if not os.path.exists(profile_path):
                continue
            
            self.log(f"\n* Профиль: {profile}")
            files_to_delete = [
                "History", "Cookies", "Login Data", "Top Sites",
                "Web Data", "Favicons", "Bookmarks"
            ]
            
            for file in files_to_delete:
                file_path = os.path.join(profile_path, file)
                if os.path.exists(file_path):
                    try:
                        os.remove(file_path)
                        self.log(f"Удалён файл: {file}")
                    except Exception as e:
                        self.log(f"Ошибка: {file} ({e})")
            
            # Очистка кеша
            cache_dir = os.path.join(profile_path, "Cache")
            if os.path.exists(cache_dir):
                shutil.rmtree(cache_dir, ignore_errors=True)
                self.log(f"Удалена папка: Cache")
    
    def clear_firefox_data(self):
        firefox_path = os.path.join(os.getenv("APPDATA"), "Mozilla\\Firefox\\Profiles")
        if not os.path.exists(firefox_path):
            self.log("[Firefox] Не найден, пропускаем...")
            return
        
        self.log("\n=== Очистка Firefox ===")
        profiles = glob.glob(os.path.join(firefox_path, "*.default*"))
        
        for profile in profiles:
            self.log(f"\n* Профиль: {os.path.basename(profile)}")
            files_to_delete = [
                "cookies.sqlite", "places.sqlite", "formhistory.sqlite",
                "webappsstore.sqlite", "favicons.sqlite"
            ]
            
            for file in files_to_delete:
                file_path = os.path.join(profile, file)
                if os.path.exists(file_path):
                    try:
                        os.remove(file_path)
                        self.log(f"Удалён файл: {file}")
                    except Exception as e:
                        self.log(f"Ошибка: {file} ({e})")
            
            cache_dir = os.path.join(profile, "cache2")
            if os.path.exists(cache_dir):
                shutil.rmtree(cache_dir, ignore_errors=True)
                self.log("Удалена папка: cache2")
    
    def run_cleaner(self):
        """Запуск очистки выбранных браузеров"""
        self.log("\nНачало очистки...")
        
        browsers = {
            "Google Chrome": ("Google\\Chrome", self.chrome_var),
            "Microsoft Edge": ("Microsoft\\Edge", self.edge_var),
            "Opera": ("Opera Software\\Opera Stable", self.opera_var),
            "Яндекс.Браузер": ("Yandex\\YandexBrowser", self.yandex_var)
        }
        
        for name, (path, var) in browsers.items():
            if var.get():
                self.clear_chrome_data(name, path)
        
        if self.firefox_var.get():
            self.clear_firefox_data()
        
        self.log("\nГотово! Данные удалены.")
        messagebox.showinfo("Готово", "Очистка завершена!")

if __name__ == "__main__":
    root = tk.Tk()
    app = BrowserCleanerApp(root)
    root.mainloop()