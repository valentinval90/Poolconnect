<img width="1235" height="448" alt="Poolconnect logo" src="https://github.com/user-attachments/assets/75dec9b7-f1db-4642-aca4-e8e317624f90" />

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

Le schéma matériel et le prototype de circuit imprimé sont déjà conçus à partir d'un **ESP32-S3-WROOM-1 (N16R8)**.

---

## 🎯 Quel problème résout-il ?

Les installations de piscines traditionnelles reposent souvent sur :  

⛔ Minuteurs de filtration qui ignorent la température de l'eau  
⛔ Aucun système d'alerte pour les fuites dans le local technique  
⛔ Aucun contrôle automatique de la pression du filtre ni du colmatage  
⛔ Aucune automatisation basée sur la météo  
⛔ Non personnalisable  
⛔ Non compatible MQTT  

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
<img width="1263" height="802" alt="image" src="https://github.com/user-attachments/assets/73d831a6-8316-4375-83a4-593155cd2701" />


## 🧪 État du logiciel

Le firmware est actuellement en phase de prototypage/test :  
- Surveillance de la température ✔  
- Première version de la logique de filtration ✔  
- Interface web ✔  
- Couche d'abstraction matérielle ✔  
- Intégration MQTT + Home Assistant ✔  
- Capteur de pression, détection de fuites et alarmes ✔    
- MAJ OTA

---
<img width="446" height="538" alt="image" src="https://github.com/user-attachments/assets/f8c1eac2-8a08-45c3-abbe-f5ee3647511e" />
<img width="1892" height="902" alt="image" src="https://github.com/user-attachments/assets/56ebca87-8b25-4ff3-93ae-a713702825d7" />
- Pour plus de visuel allez dans le dossier img du github. 
---

## 🤝 Contribuer

PoolConnect est un projet communautaire : vous pouvez contribuer en donnant votre avis, en testant des prototypes ou en participant au développement du code et de l’électronique.  
Toutes les compétences sont les bienvenues : développement logiciel, électronique, impression 3D, interface utilisateur, documentation…

J’ai choisi JLCPCB pour leur expertise et leur accompagnement tout au long du processus de production des prototypes.
Ils offrent également une excellente qualité de fabrication, une réactivité remarquable, et enfin des tarifs très attractifs !
De plus, il existe énormément d’options disponibles pour vérifier ou mesurer différents aspects des PCB : c’est un véritable plus !

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
