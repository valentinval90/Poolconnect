<img width="1536" height="1024" alt="ChatGPT Image 4 déc  2025, 11_07_22" src="https://github.com/user-attachments/assets/374204d0-3c9a-45c3-aad7-c8a686e8a70f" />


PoolConnect est un système qui rend votre piscine connectée, intelligente et open source 🔥

---

## Présentation

PoolConnect est un projet matériel et logiciel libre conçu pour faciliter la gestion des piscines, en la rendant plus intelligente, plus sûre et plus écoénergétique.  
Au lieu de s'appuyer sur des programmateurs mécaniques classiques, PoolConnect ajuste dynamiquement la filtration en fonction de la température de l’eau et des prévisions météorologiques annoncées pour la journée.  

Il permet également le contrôle de plusieurs équipements de la piscine :  
- Pompe à chaleur (contact sec)  
- Électrolyseur (contact sec)  
- Éclairage (contact sec)  
- Commande pour une électrovanne 12 V pour l’ajout d’eau automatique dans la piscine  

Le schéma matériel et le prototype de circuit imprimé sont déjà conçus à partir d'un **ESP32-S3-WROOM-1 (N16R8)**. La fabrication des prototypes en Chine est prévue prochainement. Le soutien de la communauté contribue à accélérer cette étape.

---

## 🎯 Quel problème résout-il ?

Les installations de piscines traditionnelles reposent souvent sur :  
⛔ Minuteurs de filtration qui ignorent la température de l'eau  
⛔ Aucun système d'alerte pour les fuites dans le local technique  
⛔ Aucun contrôle de la pression du filtre ni du colmatage  
⛔ Aucune automatisation basée sur la météo  

**PoolConnect** allie intelligence, connectivité et sécurité, le tout dans une solution entièrement open source.

---

## ✨ Principales caractéristiques (matériel + logiciel)

### ✔ Filtration et qualité de l'eau
- Durée de filtration automatique selon la température de l'eau et les prévisions météo  
- Température mesurée par sonde numérique DS18B20  
- Mode minuterie personnalisable (comportement similaire à une horloge programmable)  

### ✔ Sécurité et surveillance
- Entrée capteur de pression 4–20 mA pour la détection en temps réel du colmatage → seuil d'alerte configurable  
- Détecteur de fuite dans le local technique → alerte instantanée en cas d'inondation  
- Avertisseur sonore intégré (buzzer) pour les alarmes (fuite, colmatage filtre, défaut)  

### ✔ Contrôle des appareils
- Pompe (filtration)  
- Électrolyseur  
- Éclairage de piscine  
- Pompe à chaleur (commande marche/arrêt)  
- Électrovanne 12 V pour remplissage automatique  

### ✔ Connectivité et intégration
- Interface web intégrée  
- Compatibilité avec Home Assistant (intégration MQTT/native prévue)  
- Système local prioritaire : aucun cloud requis  
- API météo  

---

## 🔧 Présentation du matériel

**Microcontrôleur principal :**  
- ESP32-S3-WROOM-1 (N16R8) → 16 Mo Flash, 8 Mo PSRAM  

**Capteurs et entrées :**  
- Sonde de température numérique DS18B20  
- Capteur de pression 4–20 mA (filtration)  
- Détecteur de fuites  

**Sorties et relais :**  
- Relais pour pompe de filtration  
- Relais pour pompe à chaleur  
- Relais pour éclairage  
- Relais pour électrolyseur  
- Commande de vanne solénoïde 12 V  

**Alertes et indicateurs :**  
- Avertisseur sonore intégré  
- Protection et filtrage des entrées  
- Sélecteur de mode externe PILOTER / OFF / MANUEL pour chaque relais (interrupteur rotatif)  
- Voyants d’état externes pour chaque relais (5 V)  

---
<img width="945" height="558" alt="image" src="https://github.com/user-attachments/assets/bf2e089b-eac9-4bf1-b299-2e257974d98a" />

## 🧪 État du logiciel

Le firmware est actuellement en phase de prototypage/test :  
- Surveillance de la température ✔  
- Première version de la logique de filtration ✔  
- Interface web basique ✔  
- Couche d'abstraction matérielle en cours  
- Intégration MQTT + Home Assistant en cours  
- Capteur de pression, détection de fuites et alarmes prévus prochainement  
- Et d’autres fonctionnalités à venir  

---
<img width="444" height="508" alt="Capture d’écran 2025-12-04 105107" src="https://github.com/user-attachments/assets/c80b6ae0-70dc-4068-804d-37493614642e" />
<img width="1900" height="845" alt="Capture d’écran 2025-12-04 105415" src="https://github.com/user-attachments/assets/32e969f4-3591-4a84-afe9-c3cb77ede2a1" />

## 🚀 Feuille de route

### 🟢 Matériel
- Schéma V1  
- Prototype de circuit imprimé  
- PCB V2  
- Premier lot de prototype (Chine)  
- Intégration dans coffret électrique de piscine ([exemple Amazon](https://amzn.eu/d/afS3UlJ))  

### 🟠 Logiciel
- Réécriture complète du firmware avec dissociation des modules  
- Logique de surveillance de pression complète  
- Alarmes de détection de fuites  
- Tableau de bord dynamique (web)  
- Interface utilisateur adaptative pour mobile  
- Découverte automatique dans Home Assistant  
- Mises à jour OTA  

---

## 🤝 Contribuer

PoolConnect est un projet communautaire : vous pouvez contribuer en donnant votre avis, en testant des prototypes ou en participant au développement du code et de l’électronique.  
Toutes les compétences sont les bienvenues : développement logiciel, électronique, impression 3D, interface utilisateur, documentation…

---

## ❤️ Soutenez le projet

La prochaine étape consiste à fabriquer le circuit imprimé en Chine.  
Si PoolConnect vous inspire ou rend l'automatisation des piscines accessible, vous pouvez soutenir son développement :  

[💸 Soutenez-moi sur PayPal](https://www.paypal.com/donate/?hosted_button_id=K8RZ7FPXE39XA)  
[☕ Soutenez-moi sur Buy Me a Coffee](https://buymeacoffee.com/poolconnect)  

Votre contribution finance directement la production de matériel.

---

## 📄 Licence

Licence MIT — utilisation, modification et redistribution libres.

---

## 📬 Contactez-nous

N'hésitez pas à nous contacter ou à ouvrir un ticket pour suivre le développement ou y contribuer.  
Votre soutien accélère la production de matériel et maintient le projet entièrement open source.
