# ITMO summer practice 2026

03.06.2026-31.06.2026

Будет управление ДПТ с контуром тока и скорости. Для начала научусь пользоваться датчиком тока + ADC.

03.06: 

- написала пилу для внешнего DAC, но это не будет использоваться в итоговой задаче.

- написала функцию инициализации I2C (для current sensor) по лекции Udemy.

план на 04.06:

- взаимодействовать с current sensor по I2C, попробовать что-то считать с него.

## Как начать проект на CMSIS с помощью STM32CubeIDE:

Сначала вам нужно установить библиотеки под вашу плату (если вы первый раз создаёте проект под данную плату). Например, у меня
плата STM32F411. Директория, куда вы должны скачать их, будет User>STM32CubeIDE>Repisitory, её можно проверить при
создании проекта в Preferences, если всплывает окно с ошибкой download firmware. Не нажимайте типа "скачивать автоматически с ST.com".

Допустим, вы создали проект, не Empty, потому что иначе не подгружается папка Drivers. Но тогда у вас
автоматически сгенерируется код под HAL. Что нужно удалить, чтоб оставить минимально рабочий код?

1. .ioc можно не удалять, т.к. там удобно использовать Clock Configuration. Главное не нажимать Generate code.

2. Удалить Drivers>STM32F4xx_HAL_Driver

3. Удалить Core>Src> stm32f4xx_hal_msp.c, stm32f4xx_it.c

4. main.c, удалить всё внутри файла и вставить вместо этого:

```
#include <inttypes.h>
#include "stm32f4xx.h"

int main() {
	return 0;
}
```

5. Core>Inc> для начала можно удалить всё содержимое этой директории. Потом конечно может пригодиться заново создать пустой main.h

6. Project>Properties>C/C++ Build>Settings>MCU GCC Compiler>Preprocessor> найти переменную USE_HAL_DRIVER и удалить её, Apply
and close

7. Напишите код для включения встроенного светодиода. Например, для F411:

```
#include <inttypes.h>
#include "stm32f4xx.h"

// PC13 led on
void LED_Setup() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	// MODE13 = 01, General purpose output
	GPIOC->MODER &= ~(GPIO_MODER_MODE13);
	GPIOC->MODER |= (GPIO_MODER_MODE13_0);
}

// inverted LED logic
void LED_off() {
	GPIOC->ODR |= (GPIO_ODR_ODR_13);
}

void LED_on() {
	GPIOC->ODR &= ~(GPIO_ODR_ODR_13);
}

int main() {
	LED_Setup();
	LED_on();
	return 0;
}
```
