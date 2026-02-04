"""
POOL CONNECT INSTALLER
Application autonome pour installer les librairies Arduino ou flasher l'ESP32
Version: 0.0.1
Compatible: Windows, Linux, macOS
"""

import tkinter as tk
from tkinter import ttk, filedialog, messagebox, scrolledtext
import subprocess
import sys
import os
import platform
import json
import threading
import serial.tools.list_ports
from pathlib import Path
import requests
import zipfile
import shutil
import tarfile
import tempfile
import json

# ============================================================================
# TRADUCTIONS / TRANSLATIONS
# ============================================================================

TRANSLATIONS = {
    "fr": {
        # Interface principale
        "title": "PoolConnect Installer",
        "tab_libraries": "📚 Installer Librairies",
        "tab_flash": "⚡ Flasher ESP32",
        "tab_about": "ℹ️ À propos",
        "console_title": "📋 Console",
        
        # Onglet Librairies
        "arduino_path": "Chemin Arduino IDE",
        "browse": "Parcourir...",
        "libraries_to_install": "Librairies à installer",
        "library": "Librairie",
        "version": "Version",
        "status": "Statut",
        "not_verified": "❓ Non vérifié",
        "installed": "✅ Installé",
        "missing": "❌ Manquant",
        "btn_install_cli": "🔧 Installer Arduino CLI",
        "btn_check": "🔍 Vérifier Installation",
        "btn_install_libs": "⬇️ Installer Toutes les Librairies",
        "btn_install_esp32": "📦 Installer ESP32 Board",
        
        # Onglet Flash
        "serial_port": "Port série ESP32",
        "refresh": "🔄 Rafraîchir",
        "binary_files": "Fichiers binaires",
        "bootloader": "Bootloader (bootloader.bin):",
        "partitions": "Table partitions (partitions.bin):",
        "firmware": "Firmware (firmware.bin):",
        "filesystem": "Filesystem (littlefs.bin):",
        "btn_flash": "⚡ Flasher l'ESP32",
        "btn_erase": "🧹 Effacer Flash",
        "btn_read_info": "📖Lire Info ESP32",
        
        # Messages Console
        "initialized": "PoolConnect Installer initialisé",
        "system": "Système",
        "ports_detected": "port(s) détecté(s)",
        "no_port": "⚠️ Aucun ESP32 détecté. Vérifiez la connexion USB.",
        "arduino_not_detected": "Arduino IDE non détecté",
        
        # Installation Arduino CLI
        "cli_found": "arduino-cli trouvé",
        "cli_not_found": "arduino-cli non trouvé",
        "cli_install_auto": "Installation automatique d'arduino-cli...",
        "cli_download": "⬇️ Téléchargement depuis",
        "cli_downloading": "⏳ Téléchargement en cours...",
        "cli_downloaded": "Téléchargé",
        "cli_extract": "Extraction de l'archive...",
        "cli_installed": "arduino-cli installé",
        "cli_success_title": "Installation réussie",
        "cli_success_msg": "arduino-cli v{} installé avec succès!\n\nEmplacement: {}\n\nPour l'utiliser partout, ajoutez au PATH:\n{}",
        
        # Installation Librairies
        "checking_libs": "Vérification des librairies...",
        "installing_libs": "Début de l'installation des librairies...",
        "installing_lib": "⬇️ Installation de {} v{}...",
        "lib_installed": "{} installé avec succès",
        "installation_complete": "Installation terminée!",
        "installation_done": "Installation des librairies terminée !",
        
        # Installation ESP32
        "installing_esp32": "Installation du support ESP32...",
        "esp32_config": "⬇️ Configuration de l'index ESP32...",
        "esp32_update": "Mise à jour de l'index...",
        "esp32_install": "Installation de esp32:esp32...",
        "esp32_success": "Support ESP32 installé avec succès!",
        
        # Flash ESP32
        "flash_starting": "Début du flash de l'ESP32...",
        "flash_file": "📄",
        "flash_port": "Port",
        "flash_progress": "⏳ Flash en cours...",
        "flash_success": "Flashage terminé avec succès!",
        "flash_reboot": "Redémarrage de l'ESP32...",
        "erase_flash": "Effacement de la flash ESP32...",
        "erase_success": "Flash effacée avec succès!",
        "read_info": "Lecture des informations ESP32...",
        "esp32_info": "Informations ESP32:",
        
        # Dialogs
        "confirm": "Confirmation",
        "error": "Erreur",
        "success": "Succès",
        "cli_install_prompt": "arduino-cli n'est pas installé.\n\nVoulez-vous l'installer automatiquement ?",
        "cli_install_manual": "Installer arduino-cli automatiquement ?\n\nArduino CLI est nécessaire pour installer\nles librairies et le support ESP32.",
        "install_libs_confirm": "Installer toutes les librairies nécessaires ?",
        "install_esp32_confirm": "Installer le support ESP32 pour Arduino IDE ?",
        "flash_confirm": "Flasher l'ESP32 ?\n\nCette opération prendra 1-2 minutes.",
        "erase_confirm": "⚠️ ATTENTION ⚠️\n\nCette opération va EFFACER COMPLÈTEMENT\ntoutes les données de l'ESP32.\n\nContinuer ?",
        "flash_success_msg": "ESP32 flashé avec succès!\n\nL'ESP32 redémarre...",
        "erase_success_msg": "Flash ESP32 effacée avec succès!",
        
        # Erreurs
        "error_no_firmware": "Veuillez sélectionner le fichier firmware.bin",
        "error_no_port": "Veuillez sélectionner un port série",
        "error_flash_failed": "Échec du flashage:\n{}",
        "error_download": "Échec du téléchargement:\n{}",
        "error_install": "Échec de l'installation:\n{}",
        "error_all_failed": "Impossible de télécharger arduino-cli v{}\n\nToutes les URLs ont échoué.\n\nSolutions:\n1. Vérifiez votre connexion internet\n2. Installez manuellement depuis:\n   https://arduino.github.io/arduino-cli/latest/installation/",
        
        # À propos
        "about_text": """
        PoolConnect Installer v{}
        
        Application d'installation et de flashage pour PoolConnect
        Système de contrôle de piscine basé sur ESP32-S3
        
        📋 Fonctionnalités:
        • Installation automatique des librairies Arduino IDE
        • Installation du support ESP32 pour Arduino IDE
        • Flashage direct de l'ESP32 avec firmware complet
        • Vérification et diagnostic du matériel
        
        🔧 Matériel supporté:
        • ESP32-S3-WROOM-1-N16R8 (16MB Flash, 8MB PSRAM)
        
        📦 Librairies installées:
        {}
        
        💻 Système: {} {}
        🐍 Python: {}
        
        """
    },
    "en": {
        # Main interface
        "title": "PoolConnect Installer",
        "tab_libraries": "📚 Install Libraries",
        "tab_flash": "⚡ Flash ESP32",
        "tab_about": "ℹ️ About",
        "console_title": "📋 Console",
        
        # Libraries Tab
        "arduino_path": "Arduino IDE Path",
        "browse": "Browse...",
        "libraries_to_install": "Libraries to install",
        "library": "Library",
        "version": "Version",
        "status": "Status",
        "not_verified": "❓ Not verified",
        "installed": "✅ Installed",
        "missing": "❌ Missing",
        "btn_install_cli": "🔧 Install Arduino CLI",
        "btn_check": "🔍 Check Installation",
        "btn_install_libs": "⬇️ Install All Libraries",
        "btn_install_esp32": "📦 Install ESP32 Board",
        
        # Flash Tab
        "serial_port": "ESP32 Serial Port",
        "refresh": "🔄 Refresh",
        "binary_files": "Binary files",
        "bootloader": "Bootloader (bootloader.bin):",
        "partitions": "Partition table (partitions.bin):",
        "firmware": "Firmware (firmware.bin):",
        "filesystem": "Filesystem (littlefs.bin):",
        "btn_flash": "⚡ Flash ESP32",
        "btn_erase": "🧹 Erase Flash",
        "btn_read_info": "📖 Read ESP32 Info",
        
        # Console Messages
        "initialized": "PoolConnect Installer initialized",
        "system": "System",
        "ports_detected": "port(s) detected",
        "no_port": "⚠️ No ESP32 detected. Check USB connection.",
        "arduino_not_detected": "Arduino IDE not detected",
        
        # Arduino CLI Installation
        "cli_found": "arduino-cli found",
        "cli_not_found": "arduino-cli not found",
        "cli_install_auto": "Automatic arduino-cli installation...",
        "cli_download": "⬇Downloading from",
        "cli_downloading": "⏳ Downloading...",
        "cli_downloaded": "Downloaded",
        "cli_extract": "Extracting archive...",
        "cli_installed": "arduino-cli installed",
        "cli_success_title": "Installation successful",
        "cli_success_msg": "arduino-cli v{} installed successfully!\n\nLocation: {}\n\nTo use everywhere, add to PATH:\n{}",
        
        # Libraries Installation
        "checking_libs": "Checking libraries...",
        "installing_libs": "Starting libraries installation...",
        "installing_lib": "⬇️ Installing {} v{}...",
        "lib_installed": "{} installed successfully",
        "installation_complete": "Installation complete!",
        "installation_done": "Libraries installation complete!",
        
        # ESP32 Installation
        "installing_esp32": "Installing ESP32 support...",
        "esp32_config": "⬇️ Configuring ESP32 index...",
        "esp32_update": "Updating index...",
        "esp32_install": "Installing esp32:esp32...",
        "esp32_success": "ESP32 support installed successfully!",
        
        # Flash ESP32
        "flash_starting": "Starting ESP32 flashing...",
        "flash_file": "📄",
        "flash_port": "Port",
        "flash_progress": "⏳ Flashing in progress...",
        "flash_success": "Flashing completed successfully!",
        "flash_reboot": "Rebooting ESP32...",
        "erase_flash": "Erasing ESP32 flash...",
        "erase_success": "Flash erased successfully!",
        "read_info": "Reading ESP32 information...",
        "esp32_info": "ESP32 Information:",
        
        # Dialogs
        "confirm": "Confirmation",
        "error": "Error",
        "success": "Success",
        "cli_install_prompt": "arduino-cli is not installed.\n\nDo you want to install it automatically?",
        "cli_install_manual": "Install arduino-cli automatically?\n\nArduino CLI is required to install\nlibraries and ESP32 support.",
        "install_libs_confirm": "Install all required libraries?",
        "install_esp32_confirm": "Install ESP32 support for Arduino IDE?",
        "flash_confirm": "Flash ESP32?\n\nThis operation will take 1-2 minutes.",
        "erase_confirm": "⚠️ WARNING ⚠️\n\nThis operation will COMPLETELY ERASE\nall data on the ESP32.\n\nContinue?",
        "flash_success_msg": "ESP32 flashed successfully!\n\nESP32 is rebooting...",
        "erase_success_msg": "ESP32 flash erased successfully!",
        
        # Errors
        "error_no_firmware": "Please select the firmware.bin file",
        "error_no_port": "Please select a serial port",
        "error_flash_failed": "Flashing failed:\n{}",
        "error_download": "Download failed:\n{}",
        "error_install": "Installation failed:\n{}",
        "error_all_failed": "Unable to download arduino-cli v{}\n\nAll URLs failed.\n\nSolutions:\n1. Check your internet connection\n2. Install manually from:\n   https://arduino.github.io/arduino-cli/latest/installation/",
        
        # About
        "about_text": """
        PoolConnect Installer v{}
        
        Installation and flashing application for PoolConnect
        ESP32-S3 based pool control system
        
        📋 Features:
        • Automatic Arduino IDE libraries installation
        • ESP32 board support installation for Arduino IDE
        • Direct ESP32 flashing with complete firmware
        • Hardware verification and diagnostics
        
        🔧 Supported hardware:
        • ESP32-S3-WROOM-1-N16R8 (16MB Flash, 8MB PSRAM)
        
        📦 Installed libraries:
        {}
        
        💻 System: {} {}
        🐍 Python: {}
        """
    }
}

# ============================================================================
# CONFIGURATION
# ============================================================================

# URLs arduino-cli (version 1.0.4 - dernière stable)
ARDUINO_CLI_VERSION = "1.0.4"

# Format correct des noms de fichiers GitHub releases
ARDUINO_CLI_URLS = {
    "Windows": [
        f"https://github.com/arduino/arduino-cli/releases/download/v{ARDUINO_CLI_VERSION}/arduino-cli_{ARDUINO_CLI_VERSION}_Windows_64bit.zip",
        f"https://downloads.arduino.cc/arduino-cli/arduino-cli_{ARDUINO_CLI_VERSION}_Windows_64bit.zip"
    ],
    "Linux": [
        f"https://github.com/arduino/arduino-cli/releases/download/v{ARDUINO_CLI_VERSION}/arduino-cli_{ARDUINO_CLI_VERSION}_Linux_64bit.tar.gz",
        f"https://downloads.arduino.cc/arduino-cli/arduino-cli_{ARDUINO_CLI_VERSION}_Linux_64bit.tar.gz"
    ],
    "Darwin": [
        f"https://github.com/arduino/arduino-cli/releases/download/v{ARDUINO_CLI_VERSION}/arduino-cli_{ARDUINO_CLI_VERSION}_macOS_64bit.tar.gz",
        f"https://downloads.arduino.cc/arduino-cli/arduino-cli_{ARDUINO_CLI_VERSION}_macOS_64bit.tar.gz"
    ]
}

FIRMWARE_VERSION = "0.0.1"
ESP32_BOARD = "esp32:esp32:esp32s3"

# Librairies Arduino nécessaires
ARDUINO_LIBRARIES = {
    "WiFiManager": "2.0.16-rc.2",
    "PubSubClient": "2.8.0",
    "ArduinoJson": "6.21.3",
    "FastLED": "3.6.0",
    "OneWire": "2.3.7",
    "DallasTemperature": "3.9.0",
    "INA226": "0.5.1"
}

# Configuration ESP32
ESP32_CONFIG = {
    "flash_mode": "dio",
    "flash_freq": "80m",
    "flash_size": "16MB",
    "partition_scheme": "custom",
    "boot_loader": "0x1000",
    "partition_table": "0x8000",
    "boot_app0": "0x9000",
    "firmware": "0x10000"
}

# ============================================================================
# CLASSE PRINCIPALE
# ============================================================================

class PoolConnectInstaller:
    def __init__(self, root):
        self.root = root
        self.root.title(f"PoolConnect Installer v{FIRMWARE_VERSION}")
        self.root.geometry("800x600")
        self.root.resizable(True, True)
        
        # Langue par défaut
        self.current_language = tk.StringVar(value="fr")
        self.load_language_preference()
        
        # Variables
        self.arduino_path = tk.StringVar()
        self.selected_port = tk.StringVar()
        self.firmware_path = tk.StringVar()
        self.littlefs_path = tk.StringVar()
        self.bootloader_path = tk.StringVar()
        self.partitions_path = tk.StringVar()
        
        # Détecter Arduino IDE
        self.detect_arduino_path()
        
        # Créer l'interface
        self.create_ui()
        
        # Rafraîchir les ports
        self.refresh_ports()
    
    def t(self, key):
        """Traduit une clé selon la langue actuelle"""
        lang = self.current_language.get()
        return TRANSLATIONS.get(lang, TRANSLATIONS["fr"]).get(key, key)
    
    def load_language_preference(self):
        """Charge la préférence de langue depuis un fichier"""
        try:
            config_file = os.path.join(os.path.expanduser("~"), ".poolconnect_installer.json")
            if os.path.exists(config_file):
                with open(config_file, 'r') as f:
                    config = json.load(f)
                    self.current_language.set(config.get("language", "fr"))
        except:
            pass
    
    def save_language_preference(self):
        """Sauvegarde la préférence de langue"""
        try:
            config_file = os.path.join(os.path.expanduser("~"), ".poolconnect_installer.json")
            with open(config_file, 'w') as f:
                json.dump({"language": self.current_language.get()}, f)
        except:
            pass
    
    def change_language(self, *args):
        """Change la langue et recrée l'interface"""
        self.save_language_preference()
        # Détruire les widgets existants
        for widget in self.root.winfo_children():
            widget.destroy()
        # Recréer l'interface
        self.create_ui()
        # Rafraîchir les ports
        self.refresh_ports()
    
    # ========================================================================
    # DÉTECTION ARDUINO IDE
    # ========================================================================
    
    def detect_arduino_path(self):
        """Détecte automatiquement le chemin Arduino IDE"""
        system = platform.system()
        possible_paths = []
        
        if system == "Windows":
            possible_paths = [
                os.path.join(os.environ.get("LOCALAPPDATA", ""), "Arduino15"),
                os.path.join(os.environ.get("USERPROFILE", ""), "Documents", "Arduino"),
                "C:\\Program Files\\Arduino",
                "C:\\Program Files (x86)\\Arduino"
            ]
        elif system == "Darwin":  # macOS
            possible_paths = [
                os.path.expanduser("~/Library/Arduino15"),
                os.path.expanduser("~/Documents/Arduino"),
                "/Applications/Arduino.app"
            ]
        else:  # Linux
            possible_paths = [
                os.path.expanduser("~/.arduino15"),
                os.path.expanduser("~/Arduino"),
                "/usr/share/arduino"
            ]
        
        for path in possible_paths:
            if os.path.exists(path):
                self.arduino_path.set(path)
                return
        
        self.arduino_path.set("Arduino IDE non détecté")
    
    # ========================================================================
    # INTERFACE UTILISATEUR
    # ========================================================================
    
    def create_ui(self):
        """Crée l'interface utilisateur"""
        
        # Style
        style = ttk.Style()
        style.theme_use('clam')
        
        # En-tête avec sélecteur de langue
        header = tk.Frame(self.root, bg="#1e3a8a", height=80)
        header.pack(fill=tk.X)
        
        # Titre
        title = tk.Label(header, text=f" {self.t('title')}", 
                        font=("Arial", 24, "bold"), bg="#1e3a8a", fg="white")
        title.pack(side=tk.LEFT, padx=20, pady=20)
        
        # Sélecteur de langue (à droite)
        lang_frame = tk.Frame(header, bg="#1e3a8a")
        lang_frame.pack(side=tk.RIGHT, padx=20, pady=20)
        
        tk.Label(lang_frame, text="🌍", font=("Arial", 16), 
                bg="#1e3a8a", fg="white").pack(side=tk.LEFT, padx=5)
        
        lang_combo = ttk.Combobox(lang_frame, textvariable=self.current_language, 
                                   values=["fr", "en"], state="readonly", width=5)
        lang_combo.pack(side=tk.LEFT)
        lang_combo.bind('<<ComboboxSelected>>', self.change_language)
        
        # Notebook (onglets)
        self.notebook = ttk.Notebook(self.root)
        self.notebook.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # Onglet 1: Installation librairies
        self.tab_libraries = ttk.Frame(self.notebook)
        self.notebook.add(self.tab_libraries, text=self.t("tab_libraries"))
        self.create_libraries_tab()
        
        # Onglet 2: Flash ESP32
        self.tab_flash = ttk.Frame(self.notebook)
        self.notebook.add(self.tab_flash, text=self.t("tab_flash"))
        self.create_flash_tab()
        
        # Onglet 3: À propos
        self.tab_about = ttk.Frame(self.notebook)
        self.notebook.add(self.tab_about, text=self.t("tab_about"))
        self.create_about_tab()
        
        # Console de sortie (bas de fenêtre)
        console_frame = ttk.LabelFrame(self.root, text=self.t("console_title"), padding=10)
        console_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=(0, 10))
        
        self.console = scrolledtext.ScrolledText(console_frame, height=8, 
                                                 bg="#1e1e1e", fg="#00ff00",
                                                 font=("Consolas", 9))
        self.console.pack(fill=tk.BOTH, expand=True)
        
        self.log(self.t("initialized"))
        self.log(f"{self.t('system')}: {platform.system()} {platform.release()}")
    
    def create_libraries_tab(self):
        """Onglet installation des librairies"""
        
        # Arduino IDE Path
        path_frame = ttk.LabelFrame(self.tab_libraries, text=self.t("arduino_path"), padding=10)
        path_frame.pack(fill=tk.X, padx=10, pady=10)
        
        ttk.Entry(path_frame, textvariable=self.arduino_path, width=60).pack(side=tk.LEFT, padx=5)
        ttk.Button(path_frame, text=self.t("browse"), 
                  command=self.browse_arduino_path).pack(side=tk.LEFT)
        
        # Liste des librairies
        lib_frame = ttk.LabelFrame(self.tab_libraries, text=self.t("libraries_to_install"), padding=10)
        lib_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # Tableau des librairies
        columns = (self.t("library"), self.t("version"), self.t("status"))
        self.lib_tree = ttk.Treeview(lib_frame, columns=columns, show="headings", height=8)
        
        for col in columns:
            self.lib_tree.heading(col, text=col)
            self.lib_tree.column(col, width=200)
        
        # Ajouter les librairies
        for lib_name, version in ARDUINO_LIBRARIES.items():
            self.lib_tree.insert("", tk.END, values=(lib_name, version, self.t("not_verified")))
        
        scrollbar = ttk.Scrollbar(lib_frame, orient=tk.VERTICAL, command=self.lib_tree.yview)
        self.lib_tree.configure(yscroll=scrollbar.set)
        
        self.lib_tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        # Boutons
        btn_frame = ttk.Frame(self.tab_libraries)
        btn_frame.pack(fill=tk.X, padx=10, pady=10)
        
        ttk.Button(btn_frame, text=self.t("btn_install_cli"), 
                  command=self.install_arduino_cli_manual).pack(side=tk.LEFT, padx=5)
        ttk.Button(btn_frame, text=self.t("btn_check"), 
                  command=self.check_libraries).pack(side=tk.LEFT, padx=5)
        ttk.Button(btn_frame, text=self.t("btn_install_libs"), 
                  command=self.install_all_libraries).pack(side=tk.LEFT, padx=5)
        ttk.Button(btn_frame, text=self.t("btn_install_esp32"), 
                  command=self.install_esp32_board).pack(side=tk.LEFT, padx=5)
    
    def create_flash_tab(self):
        """Onglet flash ESP32"""
        
        # Sélection du port
        port_frame = ttk.LabelFrame(self.tab_flash, text=self.t("serial_port"), padding=10)
        port_frame.pack(fill=tk.X, padx=10, pady=10)
        
        self.port_combo = ttk.Combobox(port_frame, textvariable=self.selected_port, 
                                       width=40, state="readonly")
        self.port_combo.pack(side=tk.LEFT, padx=5)
        
        ttk.Button(port_frame, text=self.t("refresh"), 
                  command=self.refresh_ports).pack(side=tk.LEFT, padx=5)
        
        # Fichiers à flasher
        files_frame = ttk.LabelFrame(self.tab_flash, text=self.t("binary_files"), padding=10)
        files_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # Bootloader
        self.add_file_selector(files_frame, self.t("bootloader"), 
                              self.bootloader_path, row=0)
        
        # Partition table
        self.add_file_selector(files_frame, self.t("partitions"), 
                              self.partitions_path, row=1)
        
        # Firmware
        self.add_file_selector(files_frame, self.t("firmware"), 
                              self.firmware_path, row=2)
        
        # LittleFS
        self.add_file_selector(files_frame, self.t("filesystem"), 
                              self.littlefs_path, row=3)
        
        # Boutons flash
        btn_frame = ttk.Frame(self.tab_flash)
        btn_frame.pack(fill=tk.X, padx=10, pady=10)
        
        ttk.Button(btn_frame, text=self.t("btn_flash"), 
                  command=self.flash_esp32, 
                  style="Accent.TButton").pack(side=tk.LEFT, padx=5)
        ttk.Button(btn_frame, text=self.t("btn_erase"), 
                  command=self.erase_flash).pack(side=tk.LEFT, padx=5)
        ttk.Button(btn_frame, text=self.t("btn_read_info"), 
                  command=self.read_esp32_info).pack(side=tk.LEFT, padx=5)
            
    def create_about_tab(self):
        """Onglet À propos"""
        
        libs_text = "\n".join([f"  • {lib} v{ver}" for lib, ver in ARDUINO_LIBRARIES.items()])
        
        about_text = self.t("about_text").format(
            FIRMWARE_VERSION,
            libs_text,
            platform.system(),
            platform.release(),
            sys.version.split()[0]
        )
        
        about_label = tk.Label(self.tab_about, text=about_text, 
                              justify=tk.LEFT, font=("Arial", 10), 
                              bg="white", padx=20, pady=20)
        about_label.pack(fill=tk.BOTH, expand=True)
    
    def add_file_selector(self, parent, label, variable, row):
        """Ajoute un sélecteur de fichier"""
        ttk.Label(parent, text=label).grid(row=row, column=0, sticky=tk.W, pady=5)
        ttk.Entry(parent, textvariable=variable, width=50).grid(row=row, column=1, padx=5, pady=5)
        ttk.Button(parent, text="...", width=3, 
                  command=lambda: self.browse_file(variable)).grid(row=row, column=2, pady=5)
    
    # ========================================================================
    # GESTION DES PORTS
    # ========================================================================
    
    def refresh_ports(self):
        """Rafraîchit la liste des ports série"""
        ports = serial.tools.list_ports.comports()
        port_list = []
        
        for port in ports:
            # Filtrer les ports ESP32
            if "CP210" in port.description or "CH340" in port.description or "USB" in port.description:
                port_list.append(f"{port.device} - {port.description}")
        
        if port_list:
            self.port_combo['values'] = port_list
            self.port_combo.current(0)
            self.log(f"✅ {len(port_list)} {self.t('ports_detected')}")
        else:
            no_port_msg = self.t("no_port") if self.current_language.get() == "fr" else "No port detected"
            self.port_combo['values'] = [no_port_msg]
            self.log(self.t("no_port"))
    
    # ========================================================================
    # FONCTIONS NAVIGATION
    # ========================================================================
    
    def browse_arduino_path(self):
        """Parcourir pour sélectionner le chemin Arduino"""
        path = filedialog.askdirectory(title="Sélectionner le dossier Arduino IDE")
        if path:
            self.arduino_path.set(path)
    
    def browse_file(self, variable):
        """Parcourir pour sélectionner un fichier"""
        filename = filedialog.askopenfilename(
            title="Sélectionner un fichier binaire",
            filetypes=[("Fichiers binaires", "*.bin"), ("Tous les fichiers", "*.*")]
        )
        if filename:
            variable.set(filename)
    
    # ========================================================================
    # CONSOLE DE LOG
    # ========================================================================
    
    def log(self, message):
        """Affiche un message dans la console"""
        self.console.insert(tk.END, f"{message}\n")
        self.console.see(tk.END)
        self.root.update()
    
    # ========================================================================
    # INSTALLATION LIBRAIRIES
    # ========================================================================
    
    def install_arduino_cli_manual(self):
        """Installation manuelle d'arduino-cli via le bouton"""
        if messagebox.askyesno("Installer Arduino CLI", 
                              "Installer arduino-cli automatiquement ?\n\n"
                              "Arduino CLI est nécessaire pour installer\n"
                              "les librairies et le support ESP32."):
            threading.Thread(target=self._install_arduino_cli_thread, daemon=True).start()
    
    def _install_arduino_cli_thread(self):
        """Thread d'installation d'arduino-cli"""
        result = self.install_arduino_cli_auto()
        if result:
            self.log("✅ Vous pouvez maintenant installer les librairies !")
    
    def check_libraries(self):
        """Vérifie l'installation des librairies"""
        self.log(self.t("checking_libs"))
        
        arduino_cli = self.find_arduino_cli()
        if not arduino_cli:
            return
        
        for i, (lib_name, version) in enumerate(ARDUINO_LIBRARIES.items()):
            try:
                result = subprocess.run(
                    [arduino_cli, "lib", "list"],
                    capture_output=True, text=True, timeout=10
                )
                
                if lib_name in result.stdout:
                    self.lib_tree.item(self.lib_tree.get_children()[i], 
                                      values=(lib_name, version, self.t("installed")))
                    self.log(f"  {self.t('installed')} {lib_name}")
                else:
                    self.lib_tree.item(self.lib_tree.get_children()[i], 
                                      values=(lib_name, version, self.t("missing")))
                    self.log(f"  {self.t('missing')} {lib_name}")
            except Exception as e:
                self.log(f"  ⚠️ {lib_name}: {e}")
    
    def install_all_libraries(self):
        """Installe toutes les librairies"""
        if messagebox.askyesno("Confirmation", 
                              "Installer toutes les librairies nécessaires ?"):
            threading.Thread(target=self._install_libraries_thread, daemon=True).start()
    
    def _install_libraries_thread(self):
        """Thread d'installation des librairies"""
        self.log("📦 Début de l'installation des librairies...")
        
        arduino_cli = self.find_arduino_cli()
        if not arduino_cli:
            return
        
        for i, (lib_name, version) in enumerate(ARDUINO_LIBRARIES.items()):
            self.log(f"⬇️ Installation de {lib_name} v{version}...")
            
            try:
                result = subprocess.run(
                    [arduino_cli, "lib", "install", f"{lib_name}@{version}"],
                    capture_output=True, text=True, timeout=120
                )
                
                if result.returncode == 0:
                    self.lib_tree.item(self.lib_tree.get_children()[i], 
                                      values=(lib_name, version, "✅ Installé"))
                    self.log(f"  ✅ {lib_name} installé avec succès")
                else:
                    self.log(f"  ❌ Erreur: {result.stderr}")
            except Exception as e:
                self.log(f"  ❌ Erreur: {e}")
        
        self.log("✅ Installation terminée!")
        messagebox.showinfo("Installation", "Installation des librairies terminée !")
    
    def install_esp32_board(self):
        """Installe le support ESP32"""
        if messagebox.askyesno("Confirmation", 
                              "Installer le support ESP32 pour Arduino IDE ?"):
            threading.Thread(target=self._install_esp32_thread, daemon=True).start()
    
    def _install_esp32_thread(self):
        """Thread d'installation ESP32"""
        self.log("📦 Installation du support ESP32...")
        
        arduino_cli = self.find_arduino_cli()
        if not arduino_cli:
            return
        
        try:
            # Ajouter l'URL ESP32
            self.log("  ⬇️ Configuration de l'index ESP32...")
            subprocess.run(
                [arduino_cli, "config", "add", "board_manager.additional_urls",
                 "https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json"],
                capture_output=True, timeout=30
            )
            
            # Mettre à jour l'index
            self.log("  🔄 Mise à jour de l'index...")
            subprocess.run(
                [arduino_cli, "core", "update-index"],
                capture_output=True, timeout=60
            )
            
            # Installer ESP32
            self.log("  ⬇️ Installation de esp32:esp32...")
            result = subprocess.run(
                [arduino_cli, "core", "install", "esp32:esp32"],
                capture_output=True, text=True, timeout=300
            )
            
            if result.returncode == 0:
                self.log("  ✅ Support ESP32 installé avec succès!")
                messagebox.showinfo("Succès", "Support ESP32 installé avec succès!")
            else:
                self.log(f"  ❌ Erreur: {result.stderr}")
                
        except Exception as e:
            self.log(f"❌ Erreur: {e}")
    
    def find_arduino_cli(self):
        """Trouve l'exécutable arduino-cli"""
        # Essayer dans le PATH
        arduino_cli = shutil.which("arduino-cli")
        if arduino_cli:
            self.log(f"✅ arduino-cli trouvé: {arduino_cli}")
            return arduino_cli
        
        # Chercher dans les emplacements locaux
        system = platform.system()
        local_paths = []
        
        if system == "Windows":
            local_paths = [
                os.path.join(os.path.expanduser("~"), "AppData", "Local", "Arduino15", "arduino-cli.exe"),
                os.path.join(os.path.dirname(sys.executable), "arduino-cli.exe"),
                "arduino-cli.exe"
            ]
        else:
            local_paths = [
                os.path.join(os.path.expanduser("~"), ".local", "bin", "arduino-cli"),
                os.path.join(os.path.expanduser("~"), "bin", "arduino-cli"),
                "/usr/local/bin/arduino-cli",
                "arduino-cli"
            ]
        
        for path in local_paths:
            if os.path.exists(path):
                self.log(f"✅ arduino-cli trouvé: {path}")
                return path
        
        # Sinon proposer l'installation automatique
        self.log("❌ arduino-cli non trouvé")
        
        if messagebox.askyesno("Arduino CLI manquant", 
                              "arduino-cli n'est pas installé.\n\n"
                              "Voulez-vous l'installer automatiquement ?\n"
                              "(Sinon, vous pouvez l'installer manuellement)"):
            return self.install_arduino_cli_auto()
        
        return None
    
    def install_arduino_cli_auto(self):
        """Installe automatiquement arduino-cli"""
        self.log("📦 Installation automatique d'arduino-cli...")
        
        system = platform.system()
        if system not in ARDUINO_CLI_URLS:
            self.log(f"❌ Système {system} non supporté pour l'installation automatique")
            messagebox.showerror("Erreur", f"Installation automatique non supportée sur {system}")
            return None
        
        try:
            urls = ARDUINO_CLI_URLS[system]
            archive_path = None
            successful_url = None
            
            # Essayer chaque URL dans l'ordre
            for url_index, url in enumerate(urls):
                try:
                    self.log(f"⬇️ Tentative {url_index + 1}/{len(urls)}: {url}")
                    
                    # Créer un répertoire temporaire
                    with tempfile.TemporaryDirectory() as temp_dir:
                        archive_path = os.path.join(temp_dir, "arduino-cli-archive")
                        
                        self.log("⏳ Téléchargement en cours...")
                        response = requests.get(url, stream=True, timeout=30)
                        
                        # Vérifier le code HTTP
                        if response.status_code == 404:
                            self.log(f"⚠️ URL non disponible (404): {url}")
                            continue  # Essayer l'URL suivante
                        
                        response.raise_for_status()
                        
                        total_size = int(response.headers.get('content-length', 0))
                        downloaded = 0
                        
                        with open(archive_path, 'wb') as f:
                            for chunk in response.iter_content(chunk_size=8192):
                                if chunk:
                                    f.write(chunk)
                                    downloaded += len(chunk)
                                    if total_size > 0:
                                        percent = (downloaded / total_size) * 100
                                        if int(percent) % 10 == 0:  # Afficher tous les 10%
                                            self.log(f"📥 Téléchargé: {percent:.1f}%")
                                            self.root.update()
                        
                        self.log("✅ Téléchargement terminé")
                        successful_url = url
                        
                        # Extraire l'archive
                        self.log("📂 Extraction de l'archive...")
                        
                        if system == "Windows":
                            with zipfile.ZipFile(archive_path, 'r') as zip_ref:
                                zip_ref.extractall(temp_dir)
                            exe_name = "arduino-cli.exe"
                        else:
                            with tarfile.open(archive_path, 'r:gz') as tar_ref:
                                tar_ref.extractall(temp_dir)
                            exe_name = "arduino-cli"
                        
                        # Trouver l'exécutable extrait
                        extracted_exe = None
                        for root, dirs, files in os.walk(temp_dir):
                            if exe_name in files:
                                extracted_exe = os.path.join(root, exe_name)
                                break
                        
                        if not extracted_exe:
                            self.log(f"❌ Fichier {exe_name} non trouvé dans l'archive")
                            continue  # Essayer l'URL suivante
                        
                        # Déterminer le chemin d'installation
                        if system == "Windows":
                            install_dir = os.path.join(os.path.expanduser("~"), "AppData", "Local", "Arduino15")
                        else:
                            install_dir = os.path.join(os.path.expanduser("~"), ".local", "bin")
                        
                        # Créer le répertoire s'il n'existe pas
                        os.makedirs(install_dir, exist_ok=True)
                        
                        # Copier l'exécutable
                        install_path = os.path.join(install_dir, exe_name)
                        shutil.copy2(extracted_exe, install_path)
                        
                        # Rendre exécutable sur Linux/macOS
                        if system != "Windows":
                            os.chmod(install_path, 0o755)
                        
                        self.log(f"✅ arduino-cli installé: {install_path}")
                        self.log(f"✅ Source: {successful_url}")
                        
                        # Ajouter au PATH si nécessaire
                        if system == "Windows":
                            self.log("ℹ️ Pour utiliser arduino-cli partout, ajoutez au PATH:")
                            self.log(f"   {install_dir}")
                            messagebox.showinfo("Installation réussie", 
                                              f"arduino-cli v{ARDUINO_CLI_VERSION} installé avec succès!\n\n"
                                              f"Emplacement: {install_path}\n\n"
                                              f"Pour l'utiliser partout, ajoutez au PATH:\n{install_dir}")
                        else:
                            # Vérifier si ~/.local/bin est dans le PATH
                            if install_dir not in os.environ.get('PATH', ''):
                                self.log("ℹ️ Ajoutez cette ligne à ~/.bashrc ou ~/.zshrc:")
                                self.log(f'   export PATH="$PATH:{install_dir}"')
                                messagebox.showinfo("Installation réussie", 
                                                  f"arduino-cli v{ARDUINO_CLI_VERSION} installé avec succès!\n\n"
                                                  f"Emplacement: {install_path}\n\n"
                                                  f"Ajoutez à ~/.bashrc ou ~/.zshrc:\n"
                                                  f'export PATH="$PATH:{install_dir}"')
                            else:
                                messagebox.showinfo("Installation réussie", 
                                                  f"arduino-cli v{ARDUINO_CLI_VERSION} installé avec succès!\n\n"
                                                  f"Emplacement: {install_path}")
                        
                        return install_path
                        
                except requests.exceptions.RequestException as e:
                    self.log(f"⚠️ Échec tentative {url_index + 1}: {e}")
                    continue  # Essayer l'URL suivante
                except Exception as e:
                    self.log(f"⚠️ Erreur tentative {url_index + 1}: {e}")
                    continue  # Essayer l'URL suivante
            
            # Si on arrive ici, toutes les URLs ont échoué
            self.log("❌ Échec de toutes les tentatives de téléchargement")
            messagebox.showerror("Erreur", 
                               f"Impossible de télécharger arduino-cli v{ARDUINO_CLI_VERSION}\n\n"
                               f"Toutes les URLs ont échoué.\n\n"
                               f"Solutions:\n"
                               f"1. Vérifiez votre connexion internet\n"
                               f"2. Installez manuellement depuis:\n"
                               f"   https://arduino.github.io/arduino-cli/latest/installation/")
            return None
                
        except Exception as e:
            self.log(f"❌ Erreur d'installation: {e}")
            messagebox.showerror("Erreur", f"Échec de l'installation:\n{e}")
            return None
    
    # ========================================================================
    # FLASH ESP32
    # ========================================================================
    
    def flash_esp32(self):
        """Flashe l'ESP32"""
        # Vérifier les fichiers
        if not self.firmware_path.get():
            messagebox.showerror("Erreur", "Veuillez sélectionner le fichier firmware.bin")
            return
        
        if not self.selected_port.get() or "Aucun" in self.selected_port.get():
            messagebox.showerror("Erreur", "Veuillez sélectionner un port série")
            return
        
        # Vérifier les tailles des fichiers
        self.log("🔍 Vérification des fichiers binaires...")
        issues = []
        
        # Vérifier firmware (obligatoire)
        if self.firmware_path.get():
            size = os.path.getsize(self.firmware_path.get())
            self.log(f"  📄 firmware.bin: {size:,} bytes")
            if size < 100000:  # Moins de 100KB
                issues.append(f"⚠️ firmware.bin trop petit ({size} bytes)")
            elif size > 3000000:  # Plus de 3MB
                issues.append(f"⚠️ firmware.bin trop gros ({size} bytes)")
        
        # Vérifier bootloader
        if self.bootloader_path.get():
            size = os.path.getsize(self.bootloader_path.get())
            self.log(f"  📄 bootloader.bin: {size:,} bytes")
            if size < 10000 or size > 50000:
                issues.append(f"⚠️ bootloader.bin taille inhabituelle ({size} bytes)")
        
        # Vérifier partitions
        if self.partitions_path.get():
            size = os.path.getsize(self.partitions_path.get())
            self.log(f"  📄 partitions.bin: {size:,} bytes")
            if size < 100 or size > 10000:
                issues.append(f"⚠️ partitions.bin taille inhabituelle ({size} bytes)")
        
        # Vérifier littlefs
        if self.littlefs_path.get():
            size = os.path.getsize(self.littlefs_path.get())
            self.log(f"  📄 littlefs.bin: {size:,} bytes")
            if size < 1000000:  # Moins de 1MB
                issues.append(f"⚠️ littlefs.bin semble petit ({size} bytes)")
        
        if issues:
            self.log("")
            self.log("⚠️ Problèmes potentiels détectés :")
            for issue in issues:
                self.log(f"  {issue}")
            self.log("")
            if not messagebox.askyesno("Avertissement", 
                                      "\n".join(issues) + "\n\nContinuer quand même ?"):
                return
        else:
            self.log("  ✅ Tailles des fichiers OK")
        
        self.log("")
        
        if messagebox.askyesno("Confirmation", 
                              "Flasher l'ESP32 ?\n\n"
                              "Cette opération prendra 2-3 minutes.\n\n"
                              "💡 Conseil : Gardez l'ESP32 branché\n"
                              "et ne fermez pas cette fenêtre pendant\n"
                              "le flashage."):
            # Créer la fenêtre de progression
            self.create_progress_window()
            threading.Thread(target=self._flash_esp32_thread, daemon=True).start()
    
    def create_progress_window(self):
        """Crée une fenêtre de progression pour le flashage"""
        self.progress_window = tk.Toplevel(self.root)
        self.progress_window.title("Flashage ESP32 en cours...")
        self.progress_window.geometry("500x200")
        self.progress_window.resizable(False, False)
        
        # Centrer la fenêtre
        self.progress_window.transient(self.root)
        self.progress_window.grab_set()
        
        # Frame principal
        main_frame = ttk.Frame(self.progress_window, padding=20)
        main_frame.pack(fill=tk.BOTH, expand=True)
        
        # Label statut
        self.progress_label = ttk.Label(main_frame, text="Connexion à l'ESP32...", 
                                       font=("Arial", 10))
        self.progress_label.pack(pady=(0, 10))
        
        # Barre de progression
        self.progress_bar = ttk.Progressbar(main_frame, length=450, mode='determinate')
        self.progress_bar.pack(pady=10)
        
        # Label pourcentage
        self.progress_percent = ttk.Label(main_frame, text="0%", 
                                         font=("Arial", 12, "bold"))
        self.progress_percent.pack(pady=5)
        
        # Label détails
        self.progress_details = ttk.Label(main_frame, text="", 
                                         font=("Arial", 9), foreground="gray")
        self.progress_details.pack(pady=5)
        
        # Empêcher la fermeture de la fenêtre pendant le flashage
        self.progress_window.protocol("WM_DELETE_WINDOW", lambda: None)
    
    def _flash_esp32_thread(self):
        """Thread de flashage ESP32"""
        self.log("⚡ Début du flashage de l'ESP32...")
        
        # Extraire le port
        port = self.selected_port.get().split(" - ")[0]
        
        try:
            # Vérifier si esptool est installé
            try:
                subprocess.run(["python", "-m", "esptool", "version"], 
                              capture_output=True, timeout=5)
            except:
                self.log("❌ esptool non trouvé. Installation...")
                self.update_progress(5, "Installation esptool...", "📦 Téléchargement...")
                subprocess.run(["pip", "install", "esptool", "--break-system-packages"], 
                              capture_output=True, timeout=60)
            
            # Construire la commande esptool
            cmd = [
                "python", "-m", "esptool",
                "--chip", "esp32s3",
                "--port", port,
                "--baud", "460800",
                "--before", "default_reset",
                "--after", "hard_reset",
                "write_flash", "-z",
                "--flash_mode", "dio",
                "--flash_freq", "80m",
                "--flash_size", "16MB"
            ]
            
            # Compter les fichiers à flasher pour calculer la progression
            files_count = 0
            files_info = []
            
            # Ajouter bootloader si spécifié (IMPORTANT: ESP32-S3 nécessite bootloader à 0x0 !)
            if self.bootloader_path.get():
                cmd.extend(["0x0", self.bootloader_path.get()])
                files_info.append(("Bootloader", "0x0"))
                files_count += 1
                self.log(f"  📄 Bootloader (0x0): {self.bootloader_path.get()}")
            
            # Ajouter partition table si spécifiée
            if self.partitions_path.get():
                cmd.extend(["0x8000", self.partitions_path.get()])
                files_info.append(("Partitions", "0x8000"))
                files_count += 1
                self.log(f"  📄 Partitions (0x8000): {self.partitions_path.get()}")
            
            # IMPORTANT: Ajouter boot_app0 si on a les partitions (nécessaire pour l'OTA)
            if self.partitions_path.get():
                boot_app0_path = self._create_boot_app0()
                cmd.extend(["0xe000", boot_app0_path])
                files_info.append(("Boot_app0", "0xe000"))
                files_count += 1
                self.log(f"  📄 Boot_app0 (0xe000): Généré automatiquement")
            
            # Ajouter firmware (obligatoire)
            cmd.extend(["0x10000", self.firmware_path.get()])
            files_info.append(("Firmware", "0x10000"))
            files_count += 1
            self.log(f"  📄 Firmware (0x10000): {self.firmware_path.get()}")
            
            # Ajouter littlefs si spécifié
            if self.littlefs_path.get():
                cmd.extend(["0x620000", self.littlefs_path.get()])
                files_info.append(("LittleFS", "0x620000"))
                files_count += 1
                self.log(f"  📄 LittleFS (0x620000): {self.littlefs_path.get()}")
            
            self.log(f"  🔧 Port: {port}")
            self.log(f"  🔧 Vitesse: 460800 bauds")
            self.log("  ⏳ Flashage en cours (peut prendre 2-3 minutes)...")
            self.log("")
            
            self.update_progress(10, "Connexion à l'ESP32...", "🔌 Initialisation...")
            
            # Variables pour le suivi de progression
            current_file_index = 0
            base_progress = 10
            file_progress_range = 85 / files_count  # 10% à 95% répartis sur tous les fichiers
            
            # Exécuter esptool avec affichage en temps réel
            process = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                bufsize=1,
                universal_newlines=True
            )
            
            # Lire et afficher la sortie ligne par ligne EN TEMPS RÉEL
            output_lines = []
            for line in process.stdout:
                line = line.strip()
                if line:
                    output_lines.append(line)
                    
                    # Détecter connexion
                    if "Connecting" in line:
                        self.update_progress(12, "Connexion en cours...", "🔌 Établissement connexion...")
                    
                    # Détecter chip
                    elif "Chip is ESP32-S3" in line:
                        self.update_progress(15, "ESP32-S3 détecté !", "✅ Chip reconnu")
                    
                    # Détecter stub
                    elif "Stub running" in line:
                        self.update_progress(18, "Stub chargé", "⚙️ Préparation flashage...")
                    
                    # Détecter début d'écriture d'un fichier
                    elif "Writing at" in line:
                        try:
                            # Parser: "Writing at 0x00001000... (10 %)"
                            parts = line.split("Writing at")
                            if len(parts) > 1:
                                addr_str = parts[1].strip()
                                addr = addr_str.split("...")[0].strip()
                                
                                # Trouver le nom du fichier
                                file_name = "Fichier"
                                for fname, faddr in files_info:
                                    # Comparer les adresses (enlever 0x et comparer en hex)
                                    if addr.replace("0x", "").upper().startswith(faddr.replace("0x", "").upper()[:4]):
                                        file_name = fname
                                        break
                                
                                # Extraire pourcentage
                                if "(" in addr_str and "%" in addr_str:
                                    percent_str = addr_str.split("(")[1].split("%")[0].strip()
                                    file_percent = float(percent_str)
                                    
                                    # Calculer progression globale
                                    file_base = base_progress + (current_file_index * file_progress_range)
                                    file_prog = (file_percent / 100) * file_progress_range
                                    total_prog = min(95, file_base + file_prog)
                                    
                                    self.update_progress(
                                        int(total_prog),
                                        f"Flashage {file_name}...",
                                        f"📝 {file_name} à {addr}: {int(file_percent)}%"
                                    )
                        except Exception as e:
                            pass
                    
                    # Détecter fin d'un fichier
                    elif "Hash of data verified" in line:
                        if current_file_index < len(files_info):
                            file_name = files_info[current_file_index][0]
                            current_file_index += 1
                            progress = base_progress + (current_file_index * file_progress_range)
                            self.update_progress(
                                int(min(95, progress)),
                                f"✅ {file_name} flashé !",
                                f"Fichier {current_file_index}/{files_count} terminé"
                            )
                    
                    # Afficher les lignes importantes
                    if any(keyword in line for keyword in ['Writing', 'Wrote', 'Hash', 'Compressed', 
                                                           'Connecting', 'Chip is', 'MAC', 'Stub',
                                                           'Changing', 'Leaving', 'Hard resetting']):
                        self.log(f"    {line}")
                        self.root.update()
            
            # Attendre la fin du processus
            return_code = process.wait()
            result = type('obj', (object,), {
                'returncode': return_code,
                'stdout': '\n'.join(output_lines),
                'stderr': ''
            })()
            
            if result.returncode == 0:
                self.update_progress(98, "Reset de l'ESP32...", "🔄 Redémarrage...")
                
                self.log("")
                self.log("  ✅ Flashage terminé avec succès!")
                self.log("  🔄 Tentative de reset automatique...")
                
                # Attendre un peu pour que le reset se fasse
                import time
                time.sleep(2)
                
                # Forcer un reset manuel additionnel via esptool pour être sûr
                self.log("  🔄 Envoi commande de reset...")
                try:
                    reset_cmd = [
                        "python", "-m", "esptool",
                        "--chip", "esp32s3",
                        "--port", port,
                        "--after", "hard_reset",
                        "read_mac"
                    ]
                    reset_proc = subprocess.run(reset_cmd, capture_output=True, timeout=10)
                    if reset_proc.returncode == 0:
                        self.log("  ✅ Reset envoyé avec succès")
                    else:
                        self.log("  ⚠️ Reset automatique échoué (peut être normal)")
                except:
                    self.log("  ⚠️ Reset automatique échoué (peut être normal)")
                
                self.update_progress(100, "✅ Flashage terminé !", "🎉 ESP32 prêt")
                
                self.log("  ⏳ Attente démarrage ESP32 (5 secondes)...")
                time.sleep(5)
                
                # Fermer la fenêtre de progression
                if hasattr(self, 'progress_window'):
                    self.progress_window.destroy()
                
                self.log("")
                self.log("  " + "="*50)
                self.log("  ✅ FLASHAGE TERMINÉ")
                self.log("  ")
                self.log("  " + "="*50)
                
                messagebox.showinfo("Flashage Terminé", 
                                  "✅ ESP32 flashé avec succès!\n\n")
            else:
                self.update_progress(0, "❌ Erreur de flashage", "Échec")
                if hasattr(self, 'progress_window'):
                    self.progress_window.destroy()
                
                self.log(f"  ❌ Erreur: {result.stderr}")
                messagebox.showerror("Erreur", f"Échec du flashage:\n{result.stderr}")
                
        except Exception as e:
            self.update_progress(0, "❌ Erreur", str(e))
            if hasattr(self, 'progress_window'):
                self.progress_window.destroy()
            
            self.log(f"❌ Erreur: {e}")
            messagebox.showerror("Erreur", f"Erreur lors du flashage:\n{e}")
    
    def update_progress(self, percent, status, details=""):
        """Met à jour la barre de progression"""
        if hasattr(self, 'progress_window') and self.progress_window.winfo_exists():
            self.progress_bar['value'] = percent
            self.progress_percent.config(text=f"{percent}%")
            self.progress_label.config(text=status)
            if details:
                self.progress_details.config(text=details)
            self.progress_window.update()
    
    def _create_boot_app0(self):
        """Crée le fichier boot_app0.bin nécessaire pour l'ESP32-S3 OTA"""
        import tempfile
        
        # Créer un fichier temporaire
        boot_app0_path = os.path.join(tempfile.gettempdir(), "boot_app0.bin")
        
        # boot_app0.bin est un fichier de 4096 bytes (0x1000)
        # Les 32 premiers bytes définissent quelle partition OTA démarrer
        # Le reste est rempli de 0xFF
        
        with open(boot_app0_path, 'wb') as f:
            # Structure boot_app0:
            # Offset 0x00: Magic word (0xEB, 0xEB)
            # Offset 0x04: OTA sequence number (0)
            # Offset 0x08-0x0C: Reserved
            # Offset 0x10: Checksum
            # Le reste: 0xFF
            
            # Créer le buffer de 4096 bytes rempli de 0xFF
            buffer = bytearray([0xFF] * 4096)
            
            # Magic word
            buffer[0] = 0xEB
            buffer[1] = 0xEB
            buffer[2] = 0xFF
            buffer[3] = 0xFF
            
            # OTA sequence (0 = boot sur app0)
            buffer[4] = 0x00
            buffer[5] = 0x00
            buffer[6] = 0x00
            buffer[7] = 0x00
            
            # Écrire le fichier
            f.write(bytes(buffer))
        
        return boot_app0_path
    
    def erase_flash(self):
        """Efface la flash de l'ESP32"""
        if not self.selected_port.get() or "Aucun" in self.selected_port.get():
            messagebox.showerror("Erreur", "Veuillez sélectionner un port série")
            return
        
        if messagebox.askyesno("Confirmation", 
                              "⚠️ ATTENTION ⚠️\n\n"
                              "Cette opération va EFFACER COMPLÈTEMENT\n"
                              "toutes les données de l'ESP32.\n\n"
                              "Continuer ?"):
            threading.Thread(target=self._erase_flash_thread, daemon=True).start()
    
    def _erase_flash_thread(self):
        """Thread d'effacement"""
        self.log("🧹 Effacement de la flash ESP32...")
        port = self.selected_port.get().split(" - ")[0]
        
        try:
            self.log("  ⏳ Connexion à l'ESP32...")
            
            process = subprocess.Popen(
                ["python", "-m", "esptool", 
                 "--chip", "esp32s3", 
                 "--port", port,
                 "--before", "default_reset",
                 "--after", "hard_reset",
                 "erase_flash"],
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                bufsize=1,
                universal_newlines=True
            )
            
            # Afficher la sortie en temps réel
            for line in process.stdout:
                line = line.strip()
                if line:
                    if any(keyword in line for keyword in ['Connecting', 'Chip is', 'Erasing', 
                                                           'Stub', 'MAC', 'Changing']):
                        self.log(f"    {line}")
                        self.root.update()
            
            return_code = process.wait()
            
            if return_code == 0:
                self.log("")
                self.log("  ✅ Flash effacée avec succès!")
                self.log("  🔄 ESP32 redémarré automatiquement")
                self.log("")
                self.log("  💡 Prochaine étape :")
                self.log("     • Débrancher/Rebrancher l'ESP32")
                self.log("     • Puis flasher le firmware")
                messagebox.showinfo("Succès", 
                                  "Flash ESP32 effacée avec succès!\n\n"
                                  "L'ESP32 a redémarré.\n\n"
                                  "Débranchez et rebranchez l'ESP32\n"
                                  "avant de reflasher.")
            else:
                self.log(f"  ❌ Erreur lors de l'effacement")
                messagebox.showerror("Erreur", "Échec de l'effacement.\n\n"
                                     "Essayez de débrancher/rebrancher l'ESP32\n"
                                     "et recommencez.")
                
        except Exception as e:
            self.log(f"❌ Erreur: {e}")
            messagebox.showerror("Erreur", f"Erreur lors de l'effacement:\n{e}")
    
    def read_esp32_info(self):
        """Lit les informations de l'ESP32"""
        if not self.selected_port.get() or "Aucun" in self.selected_port.get():
            messagebox.showerror("Erreur", "Veuillez sélectionner un port série")
            return
        
        threading.Thread(target=self._read_esp32_info_thread, daemon=True).start()
    
    def _read_esp32_info_thread(self):
        """Thread de lecture info ESP32"""
        self.log("📖 Lecture des informations ESP32...")
        port = self.selected_port.get().split(" - ")[0]
        
        try:
            result = subprocess.run(
                ["python", "-m", "esptool", 
                 "--chip", "esp32s3", 
                 "--port", port,
                 "--before", "default_reset",
                 "--after", "no_reset",  # Pas de reset après la lecture
                 "flash_id"],
                capture_output=True, text=True, timeout=30
            )
            
            if result.returncode == 0:
                self.log("  ℹ️ Informations ESP32:")
                for line in result.stdout.split('\n'):
                    if line.strip() and not line.startswith('esptool'):
                        self.log(f"    {line}")
            else:
                self.log(f"  ❌ Erreur: {result.stderr}")
                
        except Exception as e:
            self.log(f"❌ Erreur: {e}")

# ============================================================================
# MAIN
# ============================================================================

def main():
    root = tk.Tk()
    app = PoolConnectInstaller(root)
    root.mainloop()

if __name__ == "__main__":
    main()
