// Bibliothèques
#include <PubSubClient.h> // Inclusion de la bibliothèque PubSubClient pour la communication MQTT
#include <ESP8266WiFi.h> // Inclusion de la bibliothèque ESP8266WiFi pour la connexion WiFi

// Variables
unsigned long derniereOperation; // Dernière opération effectuée (utilisée pour le délai entre les opérations)
int statusWIFI = WL_IDLE_STATUS; // Initialisation de la variable d'état WiFi en "IDLE"

// WiFi
WiFiClient clientwifi; // Création d'un client WiFi (utilisant la bibliothèque <ESP8266WiFi.h>)
#define SSID_WIFI "WiFi-QDA" // SSID du réseau WiFi auquel se connecter
#define MDP_WIFI "Qda42000!" // Mot de passe du réseau WiFi auquel se connecter

// MQTT
PubSubClient client(clientwifi); // Création d'un client MQTT (utilisant la bibliothèque <ESP8266WiFi.h>) utilisant le client WiFi créée précédemment 
char ipsrvMQTT[]  = "qda.duckdns.org"; // Adresse IP du serveur MQTT
int portsrvMQTT  = 1883; // Port du serveur MQTT
char idModuleCommande[]  = "moduleCommande01"; // Identifiant du module de commande
char topic[]  = "qda/moduleCommande01"; // Topic MQTT auquel s'abonner
char utilisateursrvMQTT[]  = "qda"; // Utilisateur du serveur MQTT
char mdpsrvMQTT[]  = "Passw0rd%1"; // Mot de passe du serveur MQTT
const String msgdesactivationVMC = "OFF"; // Message pour désactiver la VMC
const String msgactivationVMC = "ON"; // Message pour activer la VMC
const long interval_mini = 2000L; // Intervalle minimum entre deux opérations (L = long)


// Initialisation
void setup() { // Fonction d'initialisation
  pinMode(0, OUTPUT); // Initialisation du pin 0 en sortie
  digitalWrite(0, HIGH); // Initialisation du pin 0 à HIGH (désactivation de la VMC)
}

// Boucle d'éxécution du programme
void loop() { // Fonction boucle pour le fonctionnement permanent
  if (!client.connected()) { // Si le client MQTT n'est pas connecté
    connect(); // Tentative de connexion
  }
  client.loop(); // Gestion des messages MQTT entrants
}

// Configuration et connexion
void connect() { // Connexion et configuration du WiFi et du serveur MQTT
  while (!client.connected()) { // Tant que le client MQTT n'est pas connecté
    statusWIFI = WiFi.status(); // Récupération du statut de la connexion WiFi
    if ( statusWIFI != WL_CONNECTED) { // Si la connexion WiFi n'est pas établie
      WiFi.begin(SSID_WIFI, MDP_WIFI); // Tentative de connexion au réseau WiFi
      while (WiFi.status() != WL_CONNECTED) { // Attente de la connexion WiFi
        delay(500); // Attente pour ne pas saturer le processeur
      }
    }
    client.setServer(ipsrvMQTT, portsrvMQTT); // Configuration du serveur MQTT
    client.setCallback(callback); // Configuration de la fonction de rappel pour les messages MQTT entrants
    if (client.connect(idModuleCommande, utilisateursrvMQTT, mdpsrvMQTT)) { // Tentative de connexion au serveur MQTT
      client.subscribe(topic); // Abonnement au topic MQTT
    } else {
      delay(5000); // Attente avant une nouvelle tentative de connexion
    }
  }
}

// Récupération et interprétation du message
void callback(char* topic, byte* payload, unsigned int longueur) { // Fonction de callback permettant la lecture et l'interprétation du message
    signed long timestamp = millis(); // Temps actuel en millisecondes
    long deltaTime = timestamp - derniereOperation; // Temps écoulé depuis la dernière opération
    if (deltaTime > interval_mini) { // Si le délai minimum entre deux opérations est respecté
      String message = ""; // Réinitialisation de la variable message
      for (int i = 0; i < longueur; i++) { // Construction du message à partir du payload
        message = message + (char)payload[i]; // Enregistrement de chaque lettres dans la variable message
      }
      if(message == msgdesactivationVMC) { // Si le message est "OFF"
        digitalWrite(0, HIGH); // Désactivation de la VMC
        derniereOperation = timestamp; // Mise à jour du temps de la dernière opération
      } else if(message == msgactivationVMC) { // Si le message est "ON"
        digitalWrite(0, LOW); // Activation de la VMC
        derniereOperation = timestamp; // Mise à jour du temps de la dernière opération
      }
  }
}
