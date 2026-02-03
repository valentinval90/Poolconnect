// Pool Connect Pro v4.0 - Système de traductions multilingue
// Fichier de traductions FR/EN

const translations = {
  fr: {
    // === CONNEXION ===
    login_username: "Nom d'utilisateur",
    login_password: "Mot de passe",
    login_username_placeholder: "Entrez votre identifiant",
    login_password_placeholder: "Entrez votre mot de passe",
    login_button: "Se connecter",
    login_error: "Identifiants incorrects. Veuillez réessayer.",
    
    // === SIDEBAR ===
    system_active: "Système actif",
    loading: "Chargement...",
    water_temp_label: "Température de l'eau",
    recommended_time: "Temps recommandé",
    current_time: "Temps restant",
    
    // === NAVIGATION ===
    nav_home: "Accueil",
    nav_control: "Contrôle",
    nav_timers: "Timers",
    nav_calibration: "Étalonnage",
    nav_settings: "Paramètres",
    nav_users: "Utilisateurs",
    
    // === BOUTONS SIDEBAR ===
    change_password: "Changer mot de passe",
    logout: "Déconnexion",
    
    // === DASHBOARD ===
    dashboard_title: "Tableau de bord",
    water_temp: "Température Eau",
    filter_pressure: "Pression Filtre",
    ext_temp: "Température Ext.",
    system_status: "État Système",
    equipment_status: "État des équipements",
    active_timers: "Timers actifs",
    no_active_timer: "Aucun timer actif",
	no_history: "Aucun historique",
    recent_history: "Historique récent",
    
    // === ÉQUIPEMENTS ===
    eq_pump: "Pompe",
    eq_electrolyzer: "Électrolyseur",
    eq_light: "Lampe",
    eq_valve: "Électrovalve",
    eq_heatpump: "Pompe à Chaleur",
    eq_stopped: "Arrêtée",
    eq_running: "En marche",
    eq_off: "Éteinte",
    eq_on: "Allumée",
    eq_closed: "Fermée",
    eq_open: "Ouverte",
    
    // === CONTRÔLE ===
    control_title: "Contrôle manuel",
    control_subtitle: "Actionnez les équipements individuellement",
    relay_control: "Contrôle des relais",
    pump_control: "Pompe de filtration",
    electrolyzer_control: "Électrolyseur",
    light_control: "Éclairage",
    valve_control: "Électrovalve",
    heatpump_control: "Pompe à chaleur",
    turn_on: "Allumer",
    turn_off: "Éteindre",
    open: "Ouvrir",
    close: "Fermer",
    pump_desc: "Filtration et circulation de l'eau",
    electrolyzer_desc: "Production de chlore automatique",
    light_desc: "Lampe LED de la piscine",
    valve_desc: "Contrôle du remplissage d'eau",
    heatpump_desc: "Chauffage de l'eau",
    sensors: "Capteurs",
    
    // === TIMERS ===
    timers_title: "Gestion des timers",
    add_timer: "Ajouter un timer",
    edit_timer: "Modifier le timer",
    create_timer: "Créer un nouveau timer",
    no_timers: "Aucun timer configuré",
    timer_name: "Nom du timer",
    timer_enabled: "Activé",
    timer_disabled: "Désactivé",
    timer_actions: "Actions",
    delete: "Supprimer",
    edit: "Modifier",
    timers_configured: "Timers configurés",
    scenarios: "Scénarios",
    new_timer: "Nouveau Timer",
    
    // === ÉTALONNAGE ===
    calibration_title: "Étalonnage des capteurs",
    calib_water_temp: "Température de l'eau",
    calib_ext_temp: "Température ext.",
    calib_pressure: "Pression",
    calib_reset: "Réinitialiser",
    calib_assistant: "Assistant",
    calib_mode: "Mode",
    calib_offset: "Offset",
    calib_factor: "Facteur",
    calib_apply: "Appliquer",
    calib_status: "État des étalonnages",
    disabled: "Désactivé",
    current_value: "Valeur actuelle",
    launch_assistant: "Lancer assistant",
    disable: "Désactiver",
    calib_guide: "Guide d'étalonnage",
    calib_step1_title: "Préparez votre équipement de référence",
    calib_step1_desc: "Thermomètre calibré ou manomètre de précision",
    calib_step2_title: "Lancez l'assistant d'étalonnage",
    calib_step2_desc: "Suivez les instructions étape par étape",
    calib_step3_title: "Prenez vos mesures",
    calib_step3_desc: "Comparez avec votre équipement de référence",
    calib_step4_title: "Validez et testez",
    calib_step4_desc: "Vérifiez la précision après étalonnage",
    
    // === PARAMÈTRES ===
    settings_title: "Paramètres système",
    wifi_config: "Configuration WiFi",
    ssid: "SSID",
    password: "Mot de passe",
    wifi_save: "Enregistrer WiFi",
    mqtt_config: "Configuration MQTT",
    mqtt_server: "Serveur",
    mqtt_port: "Port",
    mqtt_user: "Utilisateur",
    mqtt_password: "Mot de passe",
    mqtt_save: "Enregistrer MQTT",
    system_info: "Informations système",
    firmware_version: "Version firmware",
    uptime: "Uptime",
    free_memory: "Mémoire libre",
    chart_interval: "Intervalle graphiques",
    chart_interval_desc: "Durée entre chaque mise à jour automatique des graphiques",
    chart_save: "Enregistrer",
    backup_restore: "Sauvegarde & Restauration",
    download_backup: "Télécharger backup",
    upload_backup: "Restaurer backup",
    disconnected: "Déconnecté",
    mqtt_topic: "Topic",
    test_connection: "Tester connexion",
    republish_ha: "Republier HA",
    weather_config: "Configuration Météo",
    weather_api_key: "Clé API OpenWeatherMap",
    latitude: "Latitude",
    longitude: "Longitude",
    weather_save: "Sauvegarder Météo",
    chart_config: "Configuration Graphique",
    current_interval: "Intervalle actuel",
    tips: "Conseils",
    chart_tip1: "1-2 min: Suivi détaillé à court terme",
    chart_tip2: "5 min: Équilibre idéal (recommandé)",
    chart_tip3: "10-15 min: Économie mémoire",
    chart_tip4: "30 min: Vue d'ensemble long terme",
    system_config: "Configuration Système",
    pressure_threshold: "Seuil de pression (bar)",
    pressure_alarm_desc: "Alarme si la pression dépasse ce seuil",
    buzzer: "Buzzer",
    buzzer_desc: "Activer/désactiver le buzzer d'alarme",
    system_save: "Sauvegarder Système",
    ip_address: "Adresse IP",
    ota_update: "Mise à jour OTA",
    restart: "Redémarrer",
    backup_info: "Sauvegardez votre configuration complète (utilisateurs, timers, calibration, etc.)",
    save_esp32: "Sauver sur ESP32",
    auto_backup_active: "Backup automatique quotidien activé",
    
    // === OTA ===
    ota_title: "Mises à jour OTA",
    current_partition: "Partition actuelle",
    update_partition: "Partition mise à jour",
    sketch_info: "Info Sketch",
    filesystem_info: "Info Filesystem",
    firmware_update: "Mise à jour Firmware",
    firmware_desc: "Téléchargez un nouveau firmware (.bin) pour mettre à jour le code de l'application.",
    select_firmware: "Sélectionner Firmware",
    filesystem_update: "Mise à jour Filesystem",
    filesystem_desc: "Téléchargez une image filesystem (.bin) pour mettre à jour l'interface web.",
    select_filesystem: "Sélectionner Filesystem",
    warning: "Attention",
    ota_warning: "Ne coupez pas l'alimentation pendant la mise à jour. La mise à jour peut prendre 1-2 minutes.",
    instructions: "Instructions",
    generate_firmware: "Générer firmware.bin:",
    firmware_step1: "Compilez votre sketch dans Arduino IDE",
    firmware_step2: "Allez dans Sketch → Export compiled Binary",
    firmware_step3: "Trouvez le fichier .bin dans votre dossier de projet",
    generate_filesystem: "Générer littlefs.bin:",
    filesystem_step1: "Placez vos fichiers web dans le dossier data/",
    filesystem_step2: "Utilisez l'outil ESP32 LittleFS Data Upload",
    filesystem_step3: "Ou utilisez la commande: mklittlefs -c data -p 256 -b 4096 -s 0x9E0000 littlefs.bin",
    
    // === UTILISATEURS ===
    users_title: "Gestion des utilisateurs",
    add_user: "Ajouter utilisateur",
    username: "Nom d'utilisateur",
    role: "Rôle",
    role_admin: "Administrateur",
    role_user: "Utilisateur",
    actions: "Actions",
    users_configured: "Utilisateurs configurés",
    new_user: "Nouvel utilisateur",
    admin_desc: "Les administrateurs peuvent gérer les utilisateurs",
    
    // === TIMER EDITOR ===
    timer_editor_title: "Éditeur de timer",
    general_tab: "Général",
    schedule_tab: "Planification",
    actions_tab: "Actions",
    preview_tab: "Aperçu",
    timer_name_label: "Nom du timer",
    timer_name_placeholder: "Ex: Filtration journalière",
    timer_description: "Description",
    timer_description_placeholder: "Description optionnelle",
    timer_enabled_label: "Timer activé",
    
    // === PLANIFICATION ===
    schedule_type: "Type de planification",
    schedule_daily: "Quotidien",
    schedule_weekly: "Hebdomadaire",
    schedule_interval: "Intervalle",
    schedule_cron: "CRON personnalisé",
    start_time: "Heure de début",
    days_of_week: "Jours de la semaine",
    day_monday: "Lundi",
    day_tuesday: "Mardi",
    day_wednesday: "Mercredi",
    day_thursday: "Jeudi",
    day_friday: "Vendredi",
    day_saturday: "Samedi",
    day_sunday: "Dimanche",
    interval_value: "Intervalle",
    interval_unit: "Unité",
    unit_minutes: "Minutes",
    unit_hours: "Heures",
    cron_expression: "Expression CRON",
    
    // === ACTIONS TIMER ===
    actions_list: "Liste des actions",
    add_action: "Ajouter une action",
    no_actions: "Aucune action. Cliquez sur \"Ajouter une action\" pour commencer.",
    action_relay: "Relais ON/OFF",
    action_relay_desc: "Activer ou désactiver un équipement",
    action_wait: "Attendre",
    action_wait_desc: "Pause avant l'action suivante",
    action_measure: "Mesurer Température",
    action_measure_desc: "Après 15min de pompe",
    action_auto: "Durée Auto",
    action_auto_desc: "Température / 2 heures",
    action_buzzer: "Buzzer",
    action_buzzer_desc: "Signal sonore (bips ou alarme)",
    action_led: "LED",
    action_led_desc: "Contrôle de la LED de statut",
    
    // === BOUTONS ===
    save: "Sauvegarder",
    cancel: "Annuler",
    confirm: "Confirmer",
    apply: "Appliquer",
    close: "Fermer",
    next: "Suivant",
    previous: "Précédent",
    finish: "Terminer",
    
    // === ASSISTANT ÉTALONNAGE ===
    calib_assistant_title: "Assistant Étalonnage",
    calib_mode_choice: "Choix du mode d'étalonnage",
    calib_mode_desc: "Sélectionnez la méthode d'étalonnage adaptée à vos besoins:",
    calib_offset_title: "Offset simple",
    calib_offset_desc: "Correction linéaire unique",
    calib_offset_pros: "Rapide et simple - 1 point de mesure",
    calib_offset_cons: "Moins précis sur large plage",
    calib_twopoint_title: "2 points",
    calib_twopoint_desc: "Calibration linéaire complète",
    calib_twopoint_pros: "Très précis - Compense la dérive",
    calib_twopoint_cons: "2 points de mesure",
    calib_point1: "Point de mesure 1",
    calib_point1_desc: "Prenez votre première mesure de référence",
    calib_realtime: "Valeur capteur en temps réel:",
    calib_reference: "Valeur mesurée avec votre équipement de référence:",
    calib_tip: "Conseil: Attendez que la valeur se stabilise avant de la noter",
    calib_capture: "Capturer ce point",
    calib_point2: "Point de mesure 2",
    calib_point2_warning: "Important: Changez significativement la valeur (±10°C ou ±1 bar minimum)",
    calib_validation: "Validation",
    calib_validation_desc: "Vérifiez les paramètres avant d'appliquer l'étalonnage:",
    calib_warning: "L'étalonnage sera appliqué immédiatement. Assurez-vous que les valeurs sont correctes.",
    calib_test: "Test de validation",
    calib_test_desc: "L'étalonnage a été appliqué avec succès. Test de stabilité en cours...",
    calib_testing: "Test en cours...",
    
    // === MESSAGES ===
    success: "Succès",
    error: "Erreur",
    warning: "Attention",
    info: "Information",
    backup_downloaded: "Backup téléchargé",
    backup_error: "Erreur téléchargement backup",
    backup_restored: "Backup restauré ! Le système va redémarrer dans 5 secondes...",
    backup_confirm: "ATTENTION\n\nRestaurer ce backup remplacera toute la configuration actuelle.\n\nContinuer ?",
    
    // === GRAPHIQUES ===
    daily_chart: "Graphique journalier",
    chart_temp: "Température",
    chart_pressure: "Pression",
    chart_time: "Heure",
    export_csv: "Exporter CSV",
    reset_zoom: "Réinitialiser zoom",
	  chart_date_selector: "Sélection de période",
	  previous_day: "Jour précédent",
	  next_day: "Jour suivant",
	  loading_data: "Chargement des données...",
	  today: "Aujourd'hui",
	  export_csv: "Exporter CSV",
	  storage_info: "Info stockage",
	  available_dates: "Dates disponibles",
	  loading_dates: "Chargement des dates...",
	  loading_info: "Chargement des informations...",
	  close: "Fermer",
	  no_data_for_date: "Aucune donnée pour cette date",
	  points: "points",
	  interval: "Intervalle",
	  minutes: "minutes",
    
    // === DIVERS ===
    ok: "OK",
    status_ok: "OK",
    status_error: "Erreur",
    status_warning: "Attention",
    choose_action: "Choisir une action",
    select: "Sélectionner",
    step: "Étape",
    of: "sur",
        
    // === SENSORS ===
    sensor_cover: "Volet",
    sensor_leak: "Fuite",
    sensor_buzzer: "Buzzer",
    
    // === CALIBRATION ASSISTANT ===
    calib_assistant_temp: "Assistant Étalonnage Température",
    calib_assistant_pressure: "Assistant Étalonnage Pression",
    enter_valid_value: "Veuillez entrer une valeur réelle valide",
    summary: "Récapitulatif",
    sensor: "Capteur",
    mode: "Mode",
    simple_offset: "Offset simple",
    calculated_offset: "Offset calculé",
    two_point_calib: "Calibration 2 points",
    point: "Point",
    real: "Réel",
    
    // === TIMER EDITOR ===
    timer_not_found: "Timer non trouvé",
    
    // === CONNECTION STATUS ===
    connected: "Connecté",
    
    // === SETTINGS ===
    weather_api_key_help: "Obtenez une clé gratuite sur",
    chart_interval_warning: "⚠️ Un intervalle court génère plus de points mais consomme plus de mémoire",
        
    // === CALIBRATION ASSISTANT - STEP 1 ===
    calib_mode_choice: "Choix du mode d'étalonnage",
    calib_select_method: "Sélectionnez la méthode d'étalonnage adaptée à vos besoins:",
    calib_offset_simple: "Offset simple",
    calib_offset_desc: "Correction linéaire unique",
    calib_offset_pro1: "Rapide et simple",
    calib_offset_pro2: "1 point de mesure",
    calib_offset_con: "Moins précis sur large plage",
    calib_twopoint: "2 points",
    calib_twopoint_desc: "Calibration linéaire complète",
    calib_twopoint_pro1: "Très précis",
    calib_twopoint_pro2: "Compense la dérive",
    calib_twopoint_detail: "2 points de mesure",
    
    // === CALIBRATION ASSISTANT - STEPS 2-5 ===
    calib_measure_point1: "Point de mesure 1",
    calib_measure_point2: "Point de mesure 2",
    calib_take_first_measure: "Prenez votre première mesure de référence",
    calib_realtime_value: "Valeur capteur en temps réel:",
    calib_reference_value: "Valeur mesurée avec votre équipement de référence:",
    tip: "Conseil",
    calib_wait_stabilize: "Attendez que la valeur se stabilise avant de la noter",
    calib_capture_point: "Capturer ce point",
    calib_point1_captured: "Point 1 capturé",
    important: "Important",
    calib_change_value_warning: "Changez significativement la valeur (±10°C ou ±1 bar minimum)",
    validation: "Validation",
    calib_verify_params: "Vérifiez les paramètres avant d'appliquer l'étalonnage:",
    calib_apply_warning: "L'étalonnage sera appliqué immédiatement. Assurez-vous que les valeurs sont correctes.",
    cancel: "Annuler",
    apply: "Appliquer",
    calib_validation_test: "Test de validation",
    calib_applied_success: "L'étalonnage a été appliqué avec succès. Test de stabilité en cours...",
    finish: "Terminer",
    
    // === TIMER EDITOR - TABS & CONFIG ===
    configuration: "Configuration",
    conditions: "Conditions",
    actions: "Actions",
    preview: "Aperçu",
    timer_name: "Nom du Timer",
    timer_enabled: "Timer activé",
    execution_days: "Jours d'exécution",
    start_time: "Heure de démarrage",
    fixed_time: "Heure fixe",
    sunrise: "Lever du soleil",
    sunset: "Coucher du soleil",
    offset: "Décalage",
    
    // === TIMER EDITOR - CONDITIONS ===
    timer_conditions_warning: "Le timer ne démarrera que si toutes les conditions cochées sont remplies",
    cond_cover_open: "Volet doit être ouvert",
    cond_temp_min: "Température eau minimum:",
    cond_temp_max: "Température eau maximum:",
    cond_ext_temp_min: "Température extérieure minimum:",
    cond_pressure_min: "Pression minimum:",
    
    // === HOME - DASHBOARD ===
    refresh: "Actualiser",
    points_count: "points",
    closed: "Fermé",
    leak_detected: "FUITE",
    
    // === SCENARIOS ===
    predefined_scenarios: "Scénarios Pré-configurés",
    scenario_apply_info: "Appliquez un scénario type pour démarrer rapidement",
    scenario_create_confirm: "Créer un timer depuis ce scénario ?",
    scenario_applied: "Scénario appliqué !",
    error_loading_scenarios: "Erreur chargement scénarios",
    error_applying_scenario: "Erreur application scénario",
    
    // === CHART LEGEND ===
	chart_water_temp: "Température eau",
    chart_pressure_x10: "Pression (bar x10)",
    chart_cover: "Volet",
    chart_valve: "Électrovalve",
    chart_heatpump: "PAC",
    chart_active_timers: "Timers",
    chart_pump: "Pompe",
    chart_electro: "Électrolyseur",
    chart_light: "Éclairage",

    // === TIMER ACTIONS ===
    actions_execute_order: "Actions à exécuter dans l'ordre",
    add_action: "Ajouter une action",
    no_actions_click: "Aucune action. Cliquez sur \"Ajouter une action\" pour commencer.",

    // === SCENARIOS ===
    scenario_0_name: "☀️ Été Basique",
    scenario_0_desc: "Filtration standard 8h/jour, électrolyse adaptée",
    scenario_1_name: "🔥 Été Intensif",
    scenario_1_desc: "Filtration 12h+, électrolyse renforcée, PAC active",
    scenario_2_name: "🍂 Printemps/Automne",
    scenario_2_desc: "Filtration modérée 6h, électrolyse réduite",
    scenario_3_name: "❄️ Hivernage Actif",
    scenario_3_desc: "Filtration minimale 2h, hors-gel uniquement",
    scenario_4_name: "💚 Mode Économique",
    scenario_4_desc: "Optimisé consommation, durée = Temp/2",
    scenario_5_name: "🔄 Filtration 24h",
    scenario_5_desc: "Continue avec cycles adaptés température",
    
    // === FILTRATION STATUS ===
    running: "En cours",
    stopped: "Arrêté",

    // === TIMER CONDITIONS ===
    cond_no_leak: "Pas de fuite (obligatoire)",

    // === ACTION SELECTOR ===
    choose_action: "Choisir une action",
    action_relay_title: "Relais ON/OFF",
    action_relay_desc: "Activer ou désactiver un équipement",
    action_wait_title: "Attendre",
    action_wait_desc: "Pause avant l'action suivante",
    action_measure_title: "Mesurer Température",
    action_measure_desc: "Après 15min de pompe",
    action_auto_title: "Durée Auto",
    action_auto_desc: "Température / 2 heures",
    action_buzzer_title: "Buzzer",
    action_buzzer_desc: "Signal sonore (bips ou alarme)",
    action_led_title: "LED",
    action_led_desc: "Contrôle de la LED de statut",
    
    // === TIMER DISPLAY ===
    timer_running: "En cours",
    timer_disabled: "Désactivé",
    timer_error: "Erreur",
    timer_inactive: "Inactif",

    // === TIMER LABELS ===
    state: "État",
    start_label: "Démarrage",
    days_label: "Jours",
    steps: "étape(s)",
    every_day: "Tous les jours",
    
    // === BUTTON ===
    edit: "Modifier",

    // === TIMER PREVIEW ===
    timer_unnamed: "Timer sans nom",
    cover_open: "Volet ouvert",
    water_abbr: "Eau",
    none: "Aucun",
    actions_timeline: "Timeline des actions",
    no_action_defined: "Aucune action définie",

    // === ACTION DESCRIPTIONS ===
    temp_div_2_default: "Temp / 2 (défaut, 3h-24h)",
    continuous_alarm: "Alarme continue",
    beeps: "bip(s)",
    after: "après",

    // === LED COLORS ===
    led_off: "Off",
    led_blue: "Bleu",
    led_green: "Vert",
    led_cyan: "Cyan",
    led_red: "Rouge",
    led_magenta: "Magenta",
    led_yellow: "Jaune",
    led_white: "Blanc",

    // === LED MODES ===
    led_steady: "Fixe",
    led_blinking: "Clignotant",
    led_pulsing: "Pulsation",
    
    // === UNITS SYSTEM ===
    units_preferences: "Préférences d'unités",
    temperature_unit: "Unité de température",
    pressure_unit: "Unité de pression",
    chart_pressure_psi: "Pression",
    language: "Langue"
  },
  
  en: {
    // === LOGIN ===
    login_username: "Username",
    login_password: "Password",
    login_username_placeholder: "Enter your username",
    login_password_placeholder: "Enter your password",
    login_button: "Sign in",
    login_error: "Incorrect credentials. Please try again.",
    
    // === SIDEBAR ===
    system_active: "System active",
    loading: "Loading...",
    water_temp_label: "Water temperature",
    recommended_time: "Recommended time",
    current_time: "Remaining time",
    
    // === NAVIGATION ===
    nav_home: "Home",
    nav_control: "Control",
    nav_timers: "Timers",
    nav_calibration: "Calibration",
    nav_settings: "Settings",
    nav_users: "Users",
    
    // === SIDEBAR BUTTONS ===
    change_password: "Change password",
    logout: "Logout",
    
    // === DASHBOARD ===
    dashboard_title: "Dashboard",
    water_temp: "Water Temp.",
    filter_pressure: "Filter Pressure",
    ext_temp: "External Temp.",
    system_status: "System Status",
    equipment_status: "Equipment status",
    active_timers: "Active timers",
    no_active_timer: "No active timer",
	no_history: 'No history',
    recent_history: "Recent history",
    
    // === EQUIPMENT ===
    eq_pump: "Pump",
    eq_electrolyzer: "Electrolyzer",
    eq_light: "Light",
    eq_valve: "Solenoid Valve",
    eq_heatpump: "Heat Pump",
    eq_stopped: "Stopped",
    eq_running: "Running",
    eq_off: "Off",
    eq_on: "On",
    eq_closed: "Closed",
    eq_open: "Open",
    
    // === CONTROL ===
    control_title: "Manual control",
    control_subtitle: "Operate equipment individually",
    relay_control: "Relay control",
    pump_control: "Filtration pump",
    electrolyzer_control: "Electrolyzer",
    light_control: "Lighting",
    valve_control: "Solenoid valve",
    heatpump_control: "Heat pump",
    turn_on: "Turn on",
    turn_off: "Turn off",
    open: "Open",
    close: "Close",
    pump_desc: "Filtration and water circulation",
    electrolyzer_desc: "Automatic chlorine production",
    light_desc: "Pool LED light",
    valve_desc: "Water filling control",
    heatpump_desc: "Water heating",
    sensors: "Sensors",
    
    // === TIMERS ===
    timers_title: "Timer management",
    add_timer: "Add timer",
    edit_timer: "Edit timer",
    create_timer: "Create new timer",
    no_timers: "No configured timers",
    timer_name: "Timer name",
    timer_enabled: "Enabled",
    timer_disabled: "Disabled",
    timer_actions: "Actions",
    delete: "Delete",
    edit: "Edit",
    timers_configured: "Configured timers",
    scenarios: "Scenarios",
    new_timer: "New Timer",
    
    // === CALIBRATION ===
    calibration_title: "Sensor calibration",
    calib_water_temp: "Water temp",
    calib_ext_temp: "Ext. temp.",
    calib_pressure: "Pressure",
    calib_reset: "Reset",
    calib_assistant: "Assistant",
    calib_mode: "Mode",
    calib_offset: "Offset",
    calib_factor: "Factor",
    calib_apply: "Apply",
    calib_status: "Calibration status",
    disabled: "Disabled",
    current_value: "Current value",
    launch_assistant: "Launch assistant",
    disable: "Disable",
    calib_guide: "Calibration guide",
    calib_step1_title: "Prepare your reference equipment",
    calib_step1_desc: "Calibrated thermometer or precision pressure gauge",
    calib_step2_title: "Launch calibration assistant",
    calib_step2_desc: "Follow step-by-step instructions",
    calib_step3_title: "Take your measurements",
    calib_step3_desc: "Compare with your reference equipment",
    calib_step4_title: "Validate and test",
    calib_step4_desc: "Check accuracy after calibration",
    
    // === SETTINGS ===
    settings_title: "System settings",
    wifi_config: "WiFi configuration",
    ssid: "SSID",
    password: "Password",
    wifi_save: "Save WiFi",
    mqtt_config: "MQTT configuration",
    mqtt_server: "Server",
    mqtt_port: "Port",
    mqtt_user: "User",
    mqtt_password: "Password",
    mqtt_save: "Save MQTT",
    system_info: "System information",
    firmware_version: "Firmware version",
    uptime: "Uptime",
    free_memory: "Free memory",
    chart_interval: "Chart interval",
    chart_interval_desc: "Duration between automatic chart updates",
    chart_save: "Save",
    backup_restore: "Backup & Restore",
    download_backup: "Download backup",
    upload_backup: "Restore backup",
    disconnected: "Disconnected",
    mqtt_topic: "Topic",
    test_connection: "Test connection",
    republish_ha: "Republish HA",
    weather_config: "Weather Configuration",
    weather_api_key: "OpenWeatherMap API Key",
    latitude: "Latitude",
    longitude: "Longitude",
    weather_save: "Save Weather",
    chart_config: "Chart Configuration",
    current_interval: "Current interval",
    tips: "Tips",
    chart_tip1: "1-2 min: Detailed short-term tracking",
    chart_tip2: "5 min: Ideal balance (recommended)",
    chart_tip3: "10-15 min: Memory saving",
    chart_tip4: "30 min: Long-term overview",
    system_config: "System Configuration",
    pressure_threshold: "Pressure threshold (bar)",
    pressure_alarm_desc: "Alarm if pressure exceeds this threshold",
    buzzer: "Buzzer",
    buzzer_desc: "Enable/disable alarm buzzer",
    system_save: "Save System",
    ip_address: "IP Address",
    ota_update: "OTA Update",
    restart: "Restart",
    backup_info: "Backup your complete configuration (users, timers, calibration, etc.)",
    save_esp32: "Save on ESP32",
    auto_backup_active: "Automatic daily backup enabled",
    
    // === OTA ===
    ota_title: "OTA Updates",
    current_partition: "Current Partition",
    update_partition: "Update Partition",
    sketch_info: "Sketch Info",
    filesystem_info: "Filesystem Info",
    firmware_update: "Firmware Update",
    firmware_desc: "Upload a new firmware (.bin) file to update the application code.",
    select_firmware: "Select Firmware",
    filesystem_update: "Filesystem Update",
    filesystem_desc: "Upload a filesystem image (.bin) to update the web interface.",
    select_filesystem: "Select Filesystem",
    warning: "Warning",
    ota_warning: "Do not power off the device during the update. The update may take 1-2 minutes.",
    instructions: "Instructions",
    generate_firmware: "Generate firmware.bin:",
    firmware_step1: "Compile your sketch in Arduino IDE",
    firmware_step2: "Go to Sketch → Export compiled Binary",
    firmware_step3: "Find the .bin file in your project folder",
    generate_filesystem: "Generate littlefs.bin:",
    filesystem_step1: "Place your web files in the data/ folder",
    filesystem_step2: "Use the ESP32 LittleFS Data Upload tool",
    filesystem_step3: "Or use the command: mklittlefs -c data -p 256 -b 4096 -s 0x9E0000 littlefs.bin",
    
    // === USERS ===
    users_title: "User management",
    add_user: "Add user",
    username: "Username",
    role: "Role",
    role_admin: "Administrator",
    role_user: "User",
    actions: "Actions",
    users_configured: "Configured users",
    new_user: "New user",
    admin_desc: "Administrators can manage users",
    
    // === TIMER EDITOR ===
    timer_editor_title: "Timer editor",
    general_tab: "General",
    schedule_tab: "Schedule",
    actions_tab: "Actions",
    preview_tab: "Preview",
    timer_name_label: "Timer name",
    timer_name_placeholder: "E.g: Daily filtration",
    timer_description: "Description",
    timer_description_placeholder: "Optional description",
    timer_enabled_label: "Timer enabled",
    
    // === SCHEDULE ===
    schedule_type: "Schedule type",
    schedule_daily: "Daily",
    schedule_weekly: "Weekly",
    schedule_interval: "Interval",
    schedule_cron: "Custom CRON",
    start_time: "Start time",
    days_of_week: "Days of week",
    day_monday: "Monday",
    day_tuesday: "Tuesday",
    day_wednesday: "Wednesday",
    day_thursday: "Thursday",
    day_friday: "Friday",
    day_saturday: "Saturday",
    day_sunday: "Sunday",
    interval_value: "Interval",
    interval_unit: "Unit",
    unit_minutes: "Minutes",
    unit_hours: "Hours",
    cron_expression: "CRON expression",
    
    // === TIMER ACTIONS ===
    actions_list: "Actions list",
    add_action: "Add action",
    no_actions: "No actions. Click \"Add action\" to start.",
    action_relay: "Relay ON/OFF",
    action_relay_desc: "Activate or deactivate equipment",
    action_wait: "Wait",
    action_wait_desc: "Pause before next action",
    action_measure: "Measure Temperature",
    action_measure_desc: "After 15min pump",
    action_auto: "Auto Duration",
    action_auto_desc: "Temperature / 2 hours",
    action_buzzer: "Buzzer",
    action_buzzer_desc: "Sound signal (beeps or alarm)",
    action_led: "LED",
    action_led_desc: "Status LED control",
    
    // === BUTTONS ===
    save: "Save",
    cancel: "Cancel",
    confirm: "Confirm",
    apply: "Apply",
    close: "Close",
    next: "Next",
    previous: "Previous",
    finish: "Finish",
    
    // === CALIBRATION ASSISTANT ===
    calib_assistant_title: "Calibration Assistant",
    calib_mode_choice: "Calibration mode choice",
    calib_mode_desc: "Select the calibration method suited to your needs:",
    calib_offset_title: "Simple offset",
    calib_offset_desc: "Single linear correction",
    calib_offset_pros: "Quick and simple - 1 measurement point",
    calib_offset_cons: "Less accurate over wide range",
    calib_twopoint_title: "2 points",
    calib_twopoint_desc: "Complete linear calibration",
    calib_twopoint_pros: "Very accurate - Compensates drift",
    calib_twopoint_cons: "2 measurement points",
    calib_point1: "Measurement point 1",
    calib_point1_desc: "Take your first reference measurement",
    calib_realtime: "Real-time sensor value:",
    calib_reference: "Value measured with your reference equipment:",
    calib_tip: "Tip: Wait for the value to stabilize before noting it",
    calib_capture: "Capture this point",
    calib_point2: "Measurement point 2",
    calib_point2_warning: "Important: Significantly change the value (±10°C or ±1 bar minimum)",
    calib_validation: "Validation",
    calib_validation_desc: "Check parameters before applying calibration:",
    calib_warning: "Calibration will be applied immediately. Make sure values are correct.",
    calib_test: "Validation test",
    calib_test_desc: "Calibration applied successfully. Stability test in progress...",
    calib_testing: "Testing...",
    
    // === MESSAGES ===
    success: "Success",
    error: "Error",
    warning: "Warning",
    info: "Information",
    backup_downloaded: "Backup downloaded",
    backup_error: "Backup download error",
    backup_restored: "Backup restored! System will restart in 5 seconds...",
    backup_confirm: "WARNING\n\nRestoring this backup will replace all current configuration.\n\nContinue?",
    
    // === CHARTS ===
    daily_chart: "Daily chart",
    chart_temp: "Temperature",
    chart_pressure: "Pressure",
    chart_time: "Time",
    export_csv: "Export CSV",
    reset_zoom: "Reset zoom",
	  chart_date_selector: "Period Selection",
	  previous_day: "Previous Day",
	  next_day: "Next Day",
	  loading_data: "Loading data...",
	  today: "Today",
	  export_csv: "Export CSV",
	  storage_info: "Storage Info",
	  available_dates: "Available Dates",
	  loading_dates: "Loading dates...",
	  loading_info: "Loading information...",
	  close: "Close",
	  no_data_for_date: "No data for this date",
	  points: "points",
	  interval: "Interval",
	  minutes: "minutes",
    
    // === MISC ===
    ok: "OK",
    status_ok: "OK",
    status_error: "Error",
    status_warning: "Warning",
    choose_action: "Choose action",
    select: "Select",
    step: "Step",
    of: "of",
        
    // === SENSORS ===
    sensor_cover: "Cover",
    sensor_leak: "Leak",
    sensor_buzzer: "Buzzer",
    
    // === CALIBRATION ASSISTANT ===
    calib_assistant_temp: "Temperature Calibration Assistant",
    calib_assistant_pressure: "Pressure Calibration Assistant",
    enter_valid_value: "Please enter a valid real value",
    summary: "Summary",
    sensor: "Sensor",
    mode: "Mode",
    simple_offset: "Simple offset",
    calculated_offset: "Calculated offset",
    two_point_calib: "2-point calibration",
    point: "Point",
    real: "Real",
    
    // === TIMER EDITOR ===
    timer_not_found: "Timer not found",
    
    // === CONNECTION STATUS ===
    connected: "Connected",
    
    // === SETTINGS ===
    weather_api_key_help: "Get a free key at",
    chart_interval_warning: "⚠️ A short interval generates more data points but uses more memory",
        
    // === CALIBRATION ASSISTANT - STEP 1 ===
    calib_mode_choice: "Calibration mode selection",
    calib_select_method: "Select the calibration method suited to your needs:",
    calib_offset_simple: "Simple offset",
    calib_offset_desc: "Single linear correction",
    calib_offset_pro1: "Fast and simple",
    calib_offset_pro2: "1 measurement point",
    calib_offset_con: "Less accurate over wide range",
    calib_twopoint: "2 points",
    calib_twopoint_desc: "Complete linear calibration",
    calib_twopoint_pro1: "Very accurate",
    calib_twopoint_pro2: "Compensates drift",
    calib_twopoint_detail: "2 measurement points",
    
    // === CALIBRATION ASSISTANT - STEPS 2-5 ===
    calib_measure_point1: "Measurement point 1",
    calib_measure_point2: "Measurement point 2",
    calib_take_first_measure: "Take your first reference measurement",
    calib_realtime_value: "Real-time sensor value:",
    calib_reference_value: "Value measured with your reference equipment:",
    tip: "Tip",
    calib_wait_stabilize: "Wait for the value to stabilize before noting it",
    calib_capture_point: "Capture this point",
    calib_point1_captured: "Point 1 captured",
    important: "Important",
    calib_change_value_warning: "Change the value significantly (±10°C or ±1 bar minimum)",
    validation: "Validation",
    calib_verify_params: "Verify parameters before applying calibration:",
    calib_apply_warning: "Calibration will be applied immediately. Make sure values are correct.",
    cancel: "Cancel",
    apply: "Apply",
    calib_validation_test: "Validation test",
    calib_applied_success: "Calibration applied successfully. Stability test in progress...",
    finish: "Finish",
    
    // === TIMER EDITOR - TABS & CONFIG ===
    configuration: "Configuration",
    conditions: "Conditions",
    actions: "Actions",
    preview: "Preview",
    timer_name: "Timer Name",
    timer_enabled: "Timer enabled",
    execution_days: "Execution days",
    start_time: "Start time",
    fixed_time: "Fixed time",
    sunrise: "Sunrise",
    sunset: "Sunset",
    offset: "Offset",
    
    // === TIMER EDITOR - CONDITIONS ===
    timer_conditions_warning: "Timer will only start if all checked conditions are met",
    cond_cover_open: "Cover must be open",
    cond_temp_min: "Minimum water temperature:",
    cond_temp_max: "Maximum water temperature:",
    cond_ext_temp_min: "Minimum outdoor temperature:",
    cond_pressure_min: "Minimum pressure:",
    
    // === HOME - DASHBOARD ===
    refresh: "Refresh",
    points_count: "points",
    closed: "Closed",
    leak_detected: "LEAK",
    
    // === SCENARIOS ===
    predefined_scenarios: "Predefined Scenarios",
    scenario_apply_info: "Apply a preset scenario to get started quickly",
    scenario_create_confirm: "Create a timer from this scenario?",
    scenario_applied: "Scenario applied!",
    error_loading_scenarios: "Error loading scenarios",
    error_applying_scenario: "Error applying scenario",
    
    // === CHART LEGEND ===
	chart_water_temp:"Water temperature",
    chart_pressure_x10: "Pressure (bar x10)",
    chart_cover: "Cover",
    chart_valve: "Solenoid Valve",
    chart_heatpump: "Heat Pump",
    chart_active_timers: "Timers",
    chart_pump: "Pump",
    chart_electro: "Electrolyzer",
    chart_light: "Light",

    // === TIMER ACTIONS ===
    actions_execute_order: "Actions to execute in order",
    add_action: "Add action",
    no_actions_click: "No actions. Click \"Add action\" to start.",

    // === SCENARIOS ===
	scenario_0_name: "☀️ Basic Summer",
	scenario_0_desc: "Standard filtration 8h/day, adjusted electrolysis",
	scenario_1_name: "🔥 Intensive Summer",
	scenario_1_desc: "Filtration 12h+, boosted electrolysis, heat pump active",
	scenario_2_name: "🍂 Spring/Fall",
	scenario_2_desc: "Moderate filtration 6h, reduced electrolysis",
	scenario_3_name: "❄️ Active Winterizing",
	scenario_3_desc: "Minimal filtration 2h, frost protection only",
	scenario_4_name: "💚 Eco Mode",
	scenario_4_desc: "Consumption optimized, duration = Temp/2",
	scenario_5_name: "🔄 24h Filtration",
	scenario_5_desc: "Continuous with temperature-adapted cycles",
    
    // === FILTRATION STATUS ===
    running: "Running",
    stopped: "Stopped",

    // === TIMER CONDITIONS ===
    cond_no_leak: "No leak (mandatory)",

    // === ACTION SELECTOR ===
    choose_action: "Choose an action",
    action_relay_title: "Relay ON/OFF",
    action_relay_desc: "Turn equipment on or off",
    action_wait_title: "Wait",
    action_wait_desc: "Pause before next action",
    action_measure_title: "Measure Temperature",
    action_measure_desc: "After 15min pump",
    action_auto_title: "Auto Duration",
    action_auto_desc: "Temperature / 2 hours",
    action_buzzer_title: "Buzzer",
    action_buzzer_desc: "Sound signal (beeps or alarm)",
    action_led_title: "LED",
    action_led_desc: "Status LED control",
    
    // === TIMER DISPLAY ===
    timer_running: "Running",
    timer_disabled: "Disabled",
    timer_error: "Error",
    timer_inactive: "Inactive",

    // === TIMER LABELS ===
    state: "State",
    start_label: "Start",
    days_label: "Days",
    steps: "step(s)",
    every_day: "Every day",
    
    // === BUTTON ===
    edit: "Edit",

    // === TIMER PREVIEW ===
    timer_unnamed: "Unnamed timer",
    cover_open: "Cover open",
    water_abbr: "Water",
    none: "None",
    actions_timeline: "Actions timeline",
    no_action_defined: "No action defined",

    // === ACTION DESCRIPTIONS ===
    temp_div_2_default: "Temp / 2 (default, 3h-24h)",
    continuous_alarm: "Continuous alarm",
    beeps: "beep(s)",
    after: "after",

    // === LED COLORS ===
    led_off: "Off",
    led_blue: "Blue",
    led_green: "Green",
    led_cyan: "Cyan",
    led_red: "Red",
    led_magenta: "Magenta",
    led_yellow: "Yellow",
    led_white: "White",

    // === LED MODES ===
    led_steady: "Steady",
    led_blinking: "Blinking",
    led_pulsing: "Pulsing",
    
    // === UNITS SYSTEM ===
    units_preferences: "Units preferences",
    temperature_unit: "Temperature unit",
    pressure_unit: "Pressure unit",
    chart_pressure_psi: "Pressure",
    language: "Language"
  }
};

// ============================================================================
// FONCTIONS DE GESTION DE LA LANGUE
// ============================================================================

let currentLanguage = localStorage.getItem('language') || 'fr';

function setLanguage(lang) {
  if (!translations[lang]) {
    console.error('Language not supported:', lang);
    return;
  }
  
  currentLanguage = lang;
  localStorage.setItem('language', lang);
  
  // Mettre à jour l'attribut lang du document
  document.documentElement.setAttribute('lang', lang);
  
  // Appliquer les traductions
  applyTranslations();
  
  // Sauvegarder la preference
  savePreferences();
  
  // Mettre à jour le sélecteur de langue
  updateLanguageSelector();
  
  console.log('Language changed to:', lang);
}

function t(key) {
  return translations[currentLanguage][key] || key;
}

function applyTranslations() {
  // Traduire tous les éléments avec l'attribut data-i18n
  document.querySelectorAll('[data-i18n]').forEach(element => {
    const key = element.getAttribute('data-i18n');
    
    if (element.tagName === 'INPUT' || element.tagName === 'TEXTAREA') {
      // Pour les inputs, traduire le placeholder
      if (element.hasAttribute('placeholder')) {
        element.placeholder = t(key);
      }
    } else {
      // Pour les autres éléments, traduire le contenu
      element.textContent = t(key);
    }
  });
  
  // Traduire les attributs title
  document.querySelectorAll('[data-i18n-title]').forEach(element => {
    const key = element.getAttribute('data-i18n-title');
    element.title = t(key);
  });
  
  // Traduire les placeholders
  document.querySelectorAll('[data-i18n-placeholder]').forEach(element => {
    const key = element.getAttribute('data-i18n-placeholder');
    element.placeholder = t(key);
  });
}

function updateLanguageSelector() {
  const selector = document.getElementById('languageSelector');
  if (selector) {
    selector.value = currentLanguage;
  }
}

function initLanguage() {
  // Initialiser la langue au chargement
  document.documentElement.setAttribute('lang', currentLanguage);
  updateLanguageSelector();
  // Les traductions seront appliquées après le chargement complet du DOM
}

// Initialiser la langue dès le chargement du script
if (document.readyState === 'loading') {
  document.addEventListener('DOMContentLoaded', initLanguage);
} else {
  initLanguage();
}

// ============================================================================
// EXPORT DES FONCTIONS ET VARIABLES
// ============================================================================

window.PoolTranslations = {
  translations,
  get currentLanguage() { return currentLanguage; },
  set currentLanguage(value) { 
    currentLanguage = value;
    localStorage.setItem('language', value);
  },
  t,
  setLanguage,
  applyTranslations,
  updateLanguageSelector,
  initLanguage
};

// Rétrocompatibilité
window.t = t;
window.setLanguage = setLanguage;
window.applyTranslations = applyTranslations;