.PHONY: default upload clean

default:
	platformio run --silent

upload:
	platformio run --silent --target upload

monitor:
	platformio device monitor

clean:
	platformio run --silent --target clean
