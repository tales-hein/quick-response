package mqttconn

import (
	"fmt"

	mqtt "github.com/eclipse/paho.mqtt.golang"
)

var DeviceQueue = make(chan []byte, 100)

var ReadQueue = make(chan []byte, 1000)

var messageReceivedHandler mqtt.MessageHandler = func(client mqtt.Client, msg mqtt.Message) {
	go func() {
		fmt.Printf("Received message: %s from topic: %s\n", msg.Payload(), msg.Topic())
		switch msg.Topic() {
		case "devices/":
			err := addToDeviceQueue(msg.Payload())
			if err != nil {
				fmt.Printf("Error adding to device queue: %v\n", err)
			}
		case "reading/":
			err := addToReadQueue(msg.Payload())
			if err != nil {
				fmt.Printf("Error adding to read queue: %v\n", err)
			}
		default:
			fmt.Println("Message not expected")
		}
	}()
}

var connectLostHandler mqtt.ConnectionLostHandler = func(client mqtt.Client, err error) {
	fmt.Printf("Connection lost: %v\n", err)
}

func InitMQTTClient() mqtt.Client {
	opts := mqtt.NewClientOptions()
	opts.AddBroker("tcp://localhost:1883")
	opts.SetClientID("go_mqtt_client")
	opts.OnConnectionLost = connectLostHandler

	client := mqtt.NewClient(opts)
	if token := client.Connect(); token.Wait() && token.Error() != nil {
		fmt.Printf("Error connecting to the broker: %v\n", token.Error())
		return nil
	}

	fmt.Println("Connected to the broker")

	subscribeToTopics(&client)

	return client
}

func subscribeToTopics(client *mqtt.Client) bool {
	topics := []string{"test/topic", "test/topic2"}
	for _, topic := range topics {
		token := (*client).Subscribe(topic, 0, messageReceivedHandler)
		token.Wait()

		if token.Error() != nil {
			fmt.Printf("Failed to subscribe to topic: %s, error: %v\n", topic, token.Error())
			return false
		}

		fmt.Printf("Successfully subscribed to topic: %s\n", topic)
	}
	return true
}

func publishToTopic(client *mqtt.Client, topic *string, payload *string) bool {
	token := (*client).Publish(*topic, 0, false, *payload)
	token.Wait()

	if token.Error() != nil {
		fmt.Printf("Failed to publish to topic: %s, error: %v\n", *topic, token.Error())
		return false
	}

	fmt.Printf("Successfully published to topic: %s\n", *topic)
	return true
}

func addToDeviceQueue(data []byte) error {
	select {
	case DeviceQueue <- data:
		return nil
	default:
		return fmt.Errorf("device queue is full")
	}
}

func addToReadQueue(data []byte) error {
	select {
	case ReadQueue <- data:
		return nil
	default:
		return fmt.Errorf("read queue is full")
	}
}
