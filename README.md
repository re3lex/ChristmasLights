# ChristmasLights
Это портирование проекта [ChristmasLights](https://github.com/AlexGyver/ChristmasLights) Алекса Гайвера на ESP32.

В процессе портирования "потерялась" часть функционала, ответственная за управление гирляндой по ИК и аналоговыми кнопками ¯\\_(ツ)_/¯.

## Запуск и Конфигурация
Для компилирования и прошивки нужно использовать PlatformIO.
Перед первым билдом надо скопировать `config_example.ini` в `config.ini` и поправить конфигурацию под себя. 

Также в файле `platformio.ini` надо поменять `board` на свою.