package handler

var deviceQueuePtr *chan []byte
var readQueuePtr *chan []byte

func startProcReadQueueWorker() {
	go func() {
		for msg := range *readQueuePtr {
			procReadQueue(msg)
		}
	}()
}

func startProcDeviceQueueWorker() {
	go func() {
		for msg := range *deviceQueuePtr {
			procDeviceQueue(msg)
		}
	}()
}

func InitMQTTHandler(deviceQueue *chan []byte, readQueue *chan []byte) {
	deviceQueuePtr = deviceQueue
	readQueuePtr = readQueue

	startProcReadQueueWorker()
	startProcDeviceQueueWorker()
}
