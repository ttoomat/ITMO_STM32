
#include "init.h"

int main() {
	USART2_Init();
	DAC_Init();
	while (1) {}
}
