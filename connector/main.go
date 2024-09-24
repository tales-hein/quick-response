package main

import (
	"connector/database"
	"connector/handler"
	"connector/mqttconn"
)

func main() {
	client := mqttconn.InitMQTTClient()
	if client != nil {
		handler.InitMQTTHandler(&mqttconn.DeviceQueue, &mqttconn.ReadQueue)
	}

	db := database.InitQuestDB()
}
