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
const char ipsrvMQTT[]  = ""; // Adresse IP du serveur MQTT
const int portsrvMQTT  = 1883; // Port du serveur MQTT
const char idModuleCommande[]  = ""; // Identifiant du module de commande
const char topic[]  = ""; // Topic MQTT auquel s'abonner
const char utilisateursrvMQTT[]  = ""; // Utilisateur du serveur MQTT
const char mdpsrvMQTT[]  = ""; // Mot de passe du serveur MQTT
const String msgdesactivationVMC = "OFF"; // Message pour désactiver la VMC
const String msgactivationVMC = "ON"; // Message pour activer la VMC
const long interval_mini = 2000L; // Intervalle minimum entre deux opérations (L = long)


// Initialisation
void setup() { // Fonction d'initialisation
  Serial.begin(9600); // Initialisation de la liaison série
  delay(2000); // Delais avant le démarrage pour assurer la bonne initialisation de la liaison série
  pinMode(0, OUTPUT); // Initialisation du pin 0 en sortie
  digitalWrite(0, HIGH); // Initialisation du pin 0 à HIGH (désactivation de la VMC)
  Serial.println("Paramètres du module : "); // Affichage de l'ensemble des paramètres
  Serial.print("SSID WiFi : ");
  Serial.println(SSID_WIFI);
  Serial.print("Mot de passe WiFi : ");
  Serial.println(MDP_WIFI);
  Serial.print("IP du serveur MQTT : ");
  Serial.println(ipsrvMQTT);
  Serial.print("Port du serveur MQTT : ");
  Serial.println(portsrvMQTT);
  Serial.print("ID du module de commande : ");
  Serial.println(idModuleCommande);
  Serial.print("Topic MQTT auquel s'abonner : ");
  Serial.println(topic);
  Serial.print("Nom d'utilisateur pour se connecter au serveur MQTT : ");
  Serial.println(utilisateursrvMQTT);
  Serial.print("Mot de passe pour se connecter au serveur MQTT : ");
  Serial.println(mdpsrvMQTT);
  Serial.print("Message de désactivation de la VMC : ");
  Serial.println(msgdesactivationVMC);
  Serial.print("Message d'activation de la VMC : ");
  Serial.println(msgactivationVMC);
  Serial.print("Interval minimum entre deux actions (en milisecondes) : ");
  Serial.println(interval_mini);
  Serial.println("");
  Serial.println("Initialisation terminée"); // Affichage de la fin de l'initialisation
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
  Serial.print("\nConnexion au réseau WiFi en cours ");
  while (!client.connected()) { // Tant que le client MQTT n'est pas connecté
    statusWIFI = WiFi.status(); // Récupération du statut de la connexion WiFi
    if ( statusWIFI != WL_CONNECTED) { // Si la connexion WiFi n'est pas établie
      WiFi.begin(SSID_WIFI, MDP_WIFI); // Tentative de connexion au réseau WiFi
      while (WiFi.status() != WL_CONNECTED) { // Attente de la connexion WiFi
        delay(500); // Attente pour ne pas saturer le processeur
        Serial.print(".");
      }
      Serial.print("\nConnecté au réseau WiFi ");
      Serial.println(SSID_WIFI);
    }
    client.setServer(ipsrvMQTT, portsrvMQTT); // Configuration du serveur MQTT
    client.setCallback(callback); // Configuration de la fonction de rappel pour les messages MQTT entrants
    if (client.connect(idModuleCommande, utilisateursrvMQTT, mdpsrvMQTT)) { // Tentative de connexion au serveur MQTT
      client.subscribe(topic); // Abonnement au topic MQTT
      Serial.println("Connecté au serveur MQTT");
    } else {
      Serial.print("Connexion au seerveur MQTT échouée | Message d'erreur = ");
      Serial.print(client.state());
      Serial.println(" | Nouvel essai dans 5 secondes");
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
      Serial.print("Nouveau message reçu sur le topic : ");
      Serial.println(message);
      if(message == msgdesactivationVMC) { // Si le message est "OFF"
        digitalWrite(0, HIGH); // Désactivation de la VMC
        Serial.println("Désactivation du relais VMC");
        derniereOperation = timestamp; // Mise à jour du temps de la dernière opération
      } else if(message == msgactivationVMC) { // Si le message est "ON"
        digitalWrite(0, LOW); // Activation de la VMC
        Serial.println("Activation du relais VMC");
        derniereOperation = timestamp; // Mise à jour du temps de la dernière opération
      }
  } else {
    Serial.println("Opération non effectuée, le delais minimum entre deux opérations n'est pas respecté");
  }
}
