<img width="1235" height="448" alt="Poolconnect logo" src="https://github.com/user-attachments/assets/75dec9b7-f1db-4642-aca4-e8e317624f90" />

PoolConnect est un système open-source qui rend votre piscine connectée, compatible avec Home Assistant.

---

## Présentation

**PoolConnect** est une solution open-source innovante, pensée pour rendre la gestion de votre piscine plus **intelligente, sûre et écoénergétique**.  
Oubliez les programmateurs mécaniques traditionnels : PoolConnect vous offre un contrôle **complet et personnalisable** de votre cycle de filtration.  

Vous pouvez créer votre propre équation de filtrage, allant de la plus simple (par exemple : température de l'eau ÷ 2) à des formules avancées intégrant l'ensoleillement, les prévisions météo du jour ou l'état d'ouverture du volet.

### Contrôle intelligent des équipements

Le système gère plusieurs appareils de la piscine de manière **indépendante et sécurisée**.  
Par exemple, l'électrolyseur ne peut pas s'activer si la pompe n'est pas en marche :

- Pompe à chaleur (contact sec)  
- Électrolyseur (contact sec)  
- Éclairage (contact sec)  
- Électrovanne 12 V pour l'ajout d'eau  
- Pompe de filtration  

### Surveillance et alertes

PoolConnect assure également :

- La mesure de la pression dans le filtre et l'envoi d'alertes en cas de valeurs anormales  
- La détection de fuites d'eau dans le local technique  
- La vérification de l'état (ouvert/fermé) du volet de piscine  

### Technologie

Le cœur du système est basé sur un **ESP32-S3-WROOM-1 (N16R8)**, garantissant puissance, flexibilité et connectivité pour tous vos besoins.

---

## Principales caractéristiques (matériel + logiciel)

### Filtration et qualité de l'eau

- 6 profils pré-enregistrés  
- Possibilité de créer sa propre équation pour déterminer le temps de filtration  
  *(variables disponibles : température de l'eau, température extérieure actuelle, température extérieure MAX prévue, température extérieure MIN prévue, pourcentage d'ensoleillement)*  
- Historique complet des cycles de filtration  
- Possibilité de créer des profils personnalisés de A à Z  

### Sécurité et surveillance

- Entrée capteur de pression 4–20 mA pour la détection en temps réel du colmatage du filtre, avec seuil d'alerte configurable  
  *(possibilité d'étalonnage du capteur)*  
- Détecteur de fuite dans le local technique → alerte instantanée en cas d'inondation  
- Avertisseur sonore intégré (buzzer) pour les alarmes (fuite, colmatage filtre, défaut)  
- Chaque entrée peut être utilisée pour déclencher des automatismes (coupure de la pompe, coupure de la pompe à chaleur, etc.)  

### Contrôle des appareils

- Pompe de filtration  
- Électrolyseur  
- Éclairage de piscine  
- Pompe à chaleur (commande marche/arrêt)  
- Électrovanne 12 V pour remplissage automatique  

### Connectivité et intégration

- Interface web intégrée pour configurer l'ensemble de votre PoolConnect  
- Compatibilité avec Home Assistant via intégration MQTT  
- Système entièrement local : aucun cloud requis  
- API météo intégrée  
- Enregistrement des données  
- Calibration des capteurs de pression et de température  
- Contrôle manuel via l'interface web  
- Choix de la langue (FR ou EN)  
- Configuration des utilisateurs  

---

## Présentation du matériel

### Microcontrôleur principal

- ESP32-S3-WROOM-1 (N16R8) → 16 Mo Flash, 8 Mo PSRAM  

### Capteurs et entrées

- Sonde de température numérique DS18B20  
- Capteur de pression 4–20 mA (filtration)  
- Détecteur de fuites  
- Entrée contact sec volet (ouvert / fermé)  

### Sorties et relais

- Relais pour pompe de filtration  
- Relais pour pompe à chaleur  
- Relais pour éclairage  
- Relais pour électrolyseur  
- Commande de vanne solénoïde 12 V  

### Alertes et indicateurs

- Buzzer intégré  
- Protection et filtrage des entrées  
- Sélecteur de mode externe PILOTER / OFF / MANUEL pour chaque relais (interrupteur rotatif)  
- Voyants d'état externes pour chaque relais (5 V)  

---

## Sécurité

⚠️ **ATTENTION** : Ce projet pilote des équipements électriques (230 V).  
Toute installation doit être réalisée par une personne qualifiée.  
PoolConnect ne remplace pas les protections électriques obligatoires (disjoncteur, différentiel, etc.).

---

## Réalisation avec des modules du commerce

Il est possible d'utiliser des modules indépendants pour réaliser ce projet, ou bien de fabriquer/acheter le PCB officiel (à venir).

### Liste du matériel

- [ESP32-S3-WROOM-1 (N16R8)](https://fr.aliexpress.com/item/1005007319706057.html)  
- [Sonde de température numérique DS18B20](https://fr.aliexpress.com/item/1005008024174225.html)  
- [Capteur de pression 4–20 mA](https://fr.aliexpress.com/item/1005010536806042.html)  
- [Capteur de fuite (utiliser le relais comme sortie digitale)](https://fr.aliexpress.com/item/1005001462767498.html)  
- [INA226 + résistance 1 ohm à installer](https://fr.aliexpress.com/item/1005009774355109.html)  
- [Platine de 5 relais minimum](https://fr.aliexpress.com/item/1005008269018032.html)  
- [Buzzer](https://fr.aliexpress.com/item/1005007798521103.html)  

Les éléments mentionnés ci-dessus peuvent être remplacés par des équipements équivalents.

---

## Configuration des broches (PIN OUT)

| Broche | Fonction 					 | Type 			|
|--------|-------------------------------|------------------|
| IO4    | Relais pompe filtration 		 | Sortie numérique |
| IO5    | Relais électrolyseur 		 | Sortie numérique |
| IO6    | Relais lampe 				 | Sortie numérique |
| IO7    | Relais électrovanne 			 | Sortie numérique |
| IO17   | SCL INA226 (pression 4–20 mA) | I2C 				|
| IO18   | SDA INA226 (pression 4–20 mA) | I2C 				|
| IO1    | Capteur fuite 				 | Entrée digitale  |
| IO2    | Contact ouverture volet 		 | Entrée digitale  |
| IO38   | LED adressable SK6812		 | Sortie numérique |
| IO46   | Relais pompe à chaleur 		 | Sortie numérique |
| IO14   | Data sonde température DS18B20| OneWire			|
| IO21   | Buzzer 						 | Sortie numérique |

### Schéma de câblage

Réaliser le câblage suivant :

<img width="3000" height="2177" alt="circuit_image" src="https://github.com/user-attachments/assets/56b369a0-fbc2-4f2d-930c-e06e39a72f71" />

---

## PCB du projet

Une première version du PCB est validée et fonctionnelle. Vous trouverez les fichiers pour vous permettre de commander le PCB sur internet (notamment chez JLCPCB qui m'ont soutenu pour ce projet).

Une seconde version est en cours. Cette nouvelle version a pour but de rendre le système intégrable sur le rail DIN des coffrets électriques. Elle intégrera également l'ajout de mémoire pour l'historique et quelques évolutions mineures.

### Première version

<img width="1263" height="802" alt="PCB PoolConnect V1" src="https://github.com/user-attachments/assets/73d831a6-8316-4375-83a4-593155cd2701" />

---

## Installation du firmware

Pour le téléchargement du firmware, vous pouvez utiliser Arduino IDE ou bien l'outil **"Poolconnect_installer"**.

### Avec Arduino IDE

1. Télécharger l'outil **"Poolconnect_installer"**
2. Lancer l'outil, cliquer sur **"Install All Libraries"** puis sur **"Install ESP32 Board"**  
   *(l'outil installera automatiquement les bibliothèques nécessaires pour le projet Arduino)*
3. Vous aurez également besoin de [LittleFS Upload Plugin](https://github.com/earlephilhower/arduino-littlefs-upload)

### Avec l'outil Poolconnect_installer

1. Télécharger l'outil **"Poolconnect_installer"**
2. Lancer l'outil, cliquer sur l'onglet **"Flash ESP32"**
3. Sélectionner les 4 binaires nécessaires disponibles dans le dossier **"build"** :
   - `Poolconnect.ino.bin`
   - `Poolconnect.ino.bootloader.bin`
   - `Poolconnect.ino.partitions.bin`
   - `littlefs_web_V1.0.2.bin`
4. Cliquer sur **"Flash ESP32"**

---

## 🤝 Contribuer

PoolConnect est un projet communautaire : vous pouvez contribuer en donnant votre avis, en testant ou en participant au développement du code et de l'électronique.  

Toutes les compétences sont les bienvenues :
- Développement logiciel
- Électronique
- Impression 3D
- Interface utilisateur
- Documentation
- ...

### Partenaire PCB

J'ai choisi **JLCPCB** pour leur expertise et leur accompagnement tout au long du processus de production des prototypes.

Ils offrent également :
- Une excellente qualité de fabrication
- Une réactivité remarquable
- Des tarifs très attractifs
- Énormément d'options disponibles pour vérifier ou mesurer différents aspects des PCB

---

## ❤️ Soutenez le projet

Si PoolConnect vous inspire ou rend l'automatisation des piscines accessible, vous pouvez soutenir son développement :  

- [💸 Soutenez-moi sur PayPal](https://www.paypal.com/donate/?hosted_button_id=K8RZ7FPXE39XA)  
- [☕ Soutenez-moi sur Buy Me a Coffee](https://buymeacoffee.com/poolconnect)  

---

## 📄 Licence

**Licence MIT** – utilisation, modification et redistribution libres.

Voir le fichier [LICENSE](LICENSE) pour plus de détails.

---

## 📬 Contact

N'hésitez pas à nous contacter ou à ouvrir un ticket pour suivre le développement ou y contribuer.  

Votre soutien accélère la production de matériel et maintient le projet entièrement open source.

---

**⭐ Si ce projet vous plaît, n'hésitez pas à lui donner une étoile sur GitHub !**
