.PHONY: default upload clean

default:
	platformio run --silent

upload:
	platformio run --silent --target upload

clean:
	platformio run --silent --target clean
