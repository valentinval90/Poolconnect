🌊 PoolConnect est un système qui rendra votre piscine connecter, intelligente et open source 🔥 
Présentation :
PoolConnect est un projet matériel et logiciel libre conçu pour rendre la gestion des piscines plus facile et surtout intelligente, plus sûre et plus écoénergétique. Au lieu de s'appuyer sur des programmateurs mécaniques classique, PoolConnect ajuste dynamiquement la filtration en fonction de la température de l’eau mais aussi des prévision météorologique annoncé pour la journée. Il permet également le contrôle de plusieurs équipements de la piscine, Pompe a chaleur ( contact sec ) , électrolyseur ( contact sec ), éclairage ( contact sec ) Une commande pour une electrovane en 12v est également présente  pour permettre l’ajout d’eau dans la piscine sans avoir à bouger ( grâce a un simple clic ) 
Le schéma matériel et le prototype de circuit imprimé sont déjà conçus à partir d'un ESP32-S3-WROOM-1 (N16R8), et la réalisation de prototype en Chine est prévue d’ici peu . Le soutien de la communauté contribue à accélérer cette étape.
🎯 Quel problème résout-il ?
Les installations de piscines traditionnelles reposent souvent sur :
⛔ Minuteurs de filtration qui ignorent la température de l'eau
⛔ Aucun système d'alerte pour les fuites dans le local technique de la piscine
⛔ Aucun contrôle de la pression du filtre ni du colmatage
⛔ Aucune automatisation basée sur la météo
PoolConnect allie intelligence, connectivité et sécurité, le tout dans une solution entièrement open source.
✨ Principales caractéristiques (matériel + logiciel) :
 ✔ Filtration et qualité de l'eau
Durée de filtration automatique en fonction de la température de l'eau et des condition météorologique annoncé.
Température mesurée par sonde numérique (DS18B20)
Mode minuterie personnalisée (comportement similaire à une horloge programmable)
✔ Sécurité et surveillance
Entrée capteur de pression 4–20 mA pour la détection en temps réel du colmatage du filtre → Seuil d'alerte configurable.
Détecteur de fuite d'eau dans le local technique de la piscine → Alerte instantanée en cas d'inondation du local technique
Avertisseur sonore intégré ( Buzzer )  pour les alarmes du type  (fuite, colmatage filtre, défaut)
✔ Contrôle de l'appareil
PoolConnect peut contrôler plusieurs appareils via des sorties relais :
-Pompe (filtration)
-électrolyseur
-Éclairage de piscine
-Pompe à chaleur (commande marche/arrêt)
-Électrovanne 12 V pour le remplissage automatique de piscine
✔ Connectivité et intégration
Interface web intégrée
Compatibilité avec Home Assistant (intégrations MQTT/natives prévues)
Système local prioritaire : aucun cloud requis
API météo
🔧 Présentation du matériel : 
Microcontrôleur principal :
ESP32-S3-WROOM-1 (N16R8) → 16 Mo de mémoire Flash, 8 Mo de PSRAM 
Capteurs et entrées :
Sonde de température numérique DS18B20
Entrée du capteur de pression 4–20 mA (surveillance du filtre)
Détecteur de fuites d'eau 
Sorties et relais :
Relais pour pompe de filtration
Relais pour pompe à chaleur
Relais pour éclairage
Relais pour électrolyseur
Commande de vanne solénoïde 12 V 
Alertes et indicateurs
Avertisseur sonore intégré
Protection et filtrage des entrées
System sélection mode externe  PILOTER / OFF / MANUEL pour claque relays ( Interrupteur rotatif ) 
Voyant d’état externe pour chaque Relay (5V)
  
🧪 État du logiciel
Le firmware est actuellement en phase de prototypage/test :
Surveillance de la température ✔
Première version de la logique de filtration ✔
Interface web basique ✔
Couche d'abstraction matérielle en cours de développement
Intégration MQTT + Home Assistant en cours de développement
Capteur de pression, détection de fuites et alarmes prévus ensuite
Et d’autre fonction a venir

🚀 Feuille de route 🟢 Matériel
Schéma V1
Prototype de circuit imprimé
PCB V2 
Premier lot de prototype (Chine)
Intégration prévue dans un coffret électrique de piscine type : https://amzn.eu/d/afS3UlJ
🟠 Logiciel
Réécriture complète du firmware avec dissociation des diffèrent module 
Logique de surveillance de pression complète
Alarmes de détection de fuites
Tableau de bord dynamique (web)
Interface utilisateur adaptative pour mobile
Découverte automatique de Home Assistant
Mises à jour OTA

🤝 Contribuer
PoolConnect est un projet communautaire : vous pouvez contribuer en donnant votre avis, en partageant vos idées, en testant des prototypes ou en participant au développement du code et de l’électronique. Toutes les compétences sont les bienvenues : développement logiciel, électronique, impression 3D, interface utilisateur, documentation…
❤️ Soutenez le projet
La prochaine étape consiste à fabriquer le circuit imprimé en Chine. Si PoolConnect vous inspire ou contribue à rendre l'automatisation des piscines accessible, vous pouvez soutenir son développement :
👉 Ajoutez ici vos liens de parrainage PayPal / Ko-Fi / BuyMeACoffee / GitHub
<a href=" https://www.paypal.com/donate/?hosted_button_id=K8RZ7FPXE39XA
" target="_blank">💸 Soutenez-moi sur PayPal</a>

<a href=" https://buymeacoffee.com/poolconnect " target="_blank">☕ Soutenez-moi sur Buy Me a Coffee</a>
Votre contribution finance directement la production de matériel.
📄 Licence
Licence MIT — utilisation, modification et redistribution libres.
📬 Contactez-nous
N'hésitez pas à nous contacter ou à ouvrir un ticket pour suivre le développement ou y contribuer.
Votre soutien accélère la production de matériel et contribue à maintenir le projet entièrement open source.
📄 Licence
Licence MIT — utilisation, modification et amélioration libres.
