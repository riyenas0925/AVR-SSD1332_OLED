

#include "Adafruit_GFX.h"
#include "OLED_SSD1332.h"
//#include "glcdfont.c" //comment out if you are using the custom version of Adafruit_GFX!!!!!!!!!
#include <avr/pgmspace.h>
#include "pins_arduino.h"
#include "wiring_private.h"
#include <SPI.h>



//Begin function
void OLED_SSD1332::begin(void) {
	commonInit();
	if (_inited) chipInit();
}


/***********************************/
void OLED_SSD1332::goTo(int x, int y) {
	if ((x >= WIDTH) || (y >= HEIGHT)) return;
    uint8_t cmd[] = {_CMD_SETCOLUMN,(uint8_t)x,OLED_MW,_CMD_SETROW,(uint8_t)y,OLED_MH};
    writeCommands(cmd, 6);
}

void OLED_SSD1332::goHome(void) {
	goTo(0,0);
}


uint16_t OLED_SSD1332::Color565(uint8_t r, uint8_t g, uint8_t b) {
	uint16_t c;
	c = r >> 3;
	c <<= 6;
	c |= g >> 2;
	c <<= 5;
	c |= b >> 3;
	return c;
}

//glue between Adafruit_GFX and Hardware accellerated function
void OLED_SSD1332::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color){
	hdwre_drawRect(x,y,w,h,color,false);
}

//glue between Adafruit_GFX and Hardware accellerated function
void OLED_SSD1332::fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t fillcolor) {
	hdwre_drawRect(x,y,w,h,fillcolor,true);
}

/*
hardware accellerated line draw
*/
void OLED_SSD1332::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {	
	//rotation??
	switch (getRotation()) {
		case 1:
		swap(x0, y0);
		swap(x1, y1);
		x0 = OLED_MW - x0;
		x1 = OLED_MW - x1;
		break;
	case 2:
		x0 = OLED_MW - x0;
		y0 = OLED_MH - y0;
		x1 = OLED_MW - x1;
		y1 = OLED_MH - y1;
		break;
	case 3:
		swap(x0, y0);
		swap(x1, y1);
		y0 = OLED_MH - y0;
		y1 = OLED_MH - y1;
		break;
	}
	// Boundary check
	if ((y0 >= HEIGHT) && (y1 >= HEIGHT)) return;
	if ((x0 >= WIDTH) && (x1 >= WIDTH)) return;	
	if (x0 >= WIDTH) x0 = OLED_MW;
	if (y0 >= HEIGHT) y0 = OLED_MH;
	if (x1 >= WIDTH) x1 = OLED_MW;
	if (y1 >= HEIGHT) y1 = OLED_MH;
	
	uint8_t c1 = (color & 0xF800) >> 10, c2 = (color & 0x07E0) >> 5, c3 = (color & 0x001F) << 1;//split color
	uint8_t cmds[12];//container for data
	cmds[0] = _CMD_DRAWLINE;//register
    if (x0 < x1) {
        if (y0 < y1) {
            if (_remap) _remap = remapDirection(false);
			cmds[1] = x0; cmds[2] = y0; cmds[3] = x1; cmds[4] = y1; //addressing
			cmds[5] = c1; cmds[6] = c2; cmds[7] = c3;// color
        } else if (y0 > y1) {
            if (!_remap) _remap = remapDirection(true);
			cmds[1] = OLED_MW - x1; cmds[2] = y1; cmds[3] = OLED_MW - x0; cmds[4] = y0; //addressing
			cmds[5] = c1; cmds[6] = c2; cmds[7] = c3;//color
        } else {  //  y0 == y1
            if (_remap) _remap = remapDirection(false);
			cmds[1] = x0; cmds[2] = y0; cmds[3] = x1; cmds[4] = y1; //addressing
			cmds[5] = c1; cmds[6] = c2; cmds[7] = c3;//color
        }//  end y0 == y1
	//end x0 < x1
    } else if (x0 > x1) {
        if (y0 < y1) {
            if (!_remap) _remap = remapDirection(true);
			cmds[1] = OLED_MW - x1; cmds[2] = y1; cmds[3] = OLED_MW - x0; cmds[4] = y0; //addressing
			cmds[5] = c1; cmds[6] = c2; cmds[7] = c3;//color
        } else if (y0 > y1) {
            if (_remap) _remap = remapDirection(false);
			cmds[1] = x0; cmds[2] = y0; cmds[3] = x1; cmds[4] = y1; //addressing
			cmds[5] = c1; cmds[6] = c2; cmds[7] = c3;//color
        } else {  //  y0 == y1
			//bitClear(_remapData,1);//change remap bit
			//setRegister(_CMD_SETREMAP,_remapData);//0b01110000
			if (_remap) _remap = remapDirection(false);
			cmds[1] = OLED_MW - x1; cmds[2] = y1; cmds[3] = OLED_MW - x0; cmds[4] = y0; //addressing
			cmds[5] = c1; cmds[6] = c2; cmds[7] = c3;//color
			//writeCommands(cmds, 8);//send
			//bitSet(_remapData,1);//change remap bit
			//setRegister(_CMD_SETREMAP,_remapData);//0b01110010
        }// end y0 == y1
	// end x0 > x1
    } else {  //  x0 == x1
        if (y0 < y1) {
            if (_remap) _remap = remapDirection(false);
			cmds[1] = x0; cmds[2] = y0; cmds[3] = x1; cmds[4] = y1; //addressing
			cmds[5] = c1; cmds[6] = c2; cmds[7] = c3;//color
        } else if (y0 > y1) {
			cmds[1] = x1; cmds[2] = y1; cmds[3] = x0; cmds[4] = y0; //addressing
			cmds[5] = c1; cmds[6] = c2; cmds[7] = c3;//color
        } else {  //  y0 == y1
            if (_remap) _remap = remapDirection(false);
			cmds[1] = x0; cmds[2] = y0; cmds[3] = x1; cmds[4] = y1; //addressing
			cmds[5] = c1; cmds[6] = c2; cmds[7] = c3;//color
        }//  end y0 == y1
	//  end x0 == x1
    }//end  x1 == x2
	writeCommands(cmds, 8); //send
	delayMicroseconds(_DLY_LINE);
}

/*
Hardware accellerated pixel set by drawing a 1 pixel rectangle
*/
void OLED_SSD1332::drawPixel(int16_t x, int16_t y, uint16_t color){
	if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;
	// check rotation, move pixel around if necessary
	switch (getRotation()) {
	case 1:
		swap(x, y);
		//x = OLED_MW - x;//HH2
		break;
	case 2:
		x = OLED_MW - x;
		y = OLED_MH - y;
		break;
	case 3:
		swap(x, y);
		y = OLED_MH - y;
		break;
	}
	uint8_t c1 = (color & 0xf800) >> 10, c2 = (color & 0x07e0) >> 5, c3 = (color & 0x001f) << 1;
    uint8_t cmd[11] = {_CMD_DRAWRECT, (uint8_t)x, (uint8_t)y, (uint8_t)x, (uint8_t)y, c1, c2, c3, c1, c2, c3};
    writeCommands(cmd, 11);
}

//push 16bit color
void OLED_SSD1332::pushColor(uint16_t color) {
  // setup for data
	uint8_t cmd[2] = {(uint8_t)(color >> 8), (uint8_t)color};
	writeCommands(cmd, 2);
}

//Hardware accellerated clear screen based on fillRect
void OLED_SSD1332::clearScreen(int16_t color) {
	if (color == -1){
		uint8_t cmd[5] = {_CMD_CLRWINDOW,0,0,(uint8_t)(WIDTH),(uint8_t)(HEIGHT)};
		writeCommands(cmd, 5);
	} else {
		hdwre_drawRect(0,0,(uint8_t)(width()),(uint8_t)(height()),color,true);
		//hdwre_drawRect(0,0,(uint8_t)(WIDTH),(uint8_t)(HEIGHT),color,true);
	}
}


//thanks fxmech for fix this!
void OLED_SSD1332::setRotation(uint8_t x) {
	rotation = (x & 3);
	switch(rotation) {
	case 0:
		_width = WIDTH;
		_height = HEIGHT;
		bitClear(_remapData,4);
	break;
	case 1:
		_width = HEIGHT;
		_height = WIDTH;
		bitSet(_remapData,1);
	break;
	case 2:
		_width = WIDTH;
		_height = HEIGHT;
		bitSet(_remapData,4);
	break;
	case 3:
		_width = HEIGHT;
		_height = WIDTH;
		bitClear(_remapData,1);
	break;
	}
	setRegister(_CMD_SETREMAP,_remapData);
}

/*
0: fixed:Normal - pixel:0 it's the bottom, 1 it's the top
1: fixed:Text mirrored
2: fixed:Not drawing
3: Correct
*/
/*
void OLED_SSD1332::test(int8_t count){
	int c,i;
	int start;
	int stop;
	if (count < 65){
		start = count;
		stop = count+1;
	} else {
		start = 0;
		stop = height();
	}
	drawPixel(width()/2,height()/2,0xFFE0);
	delay(1000);
	for (c=start;c<stop;c++){
		setCursor(width()/2,height()/2);
		setTextColor(0x0000, 0x0000);
		print(88);
		setCursor(width()/2,height()/2);
		if (c == 0){
			setTextColor(0xF81F, 0x0000);
		} else {
			setTextColor(0xFFFF, 0x0000);
		}
		print(c,DEC);
		for (i=0;i<width();i++){
			if (c > 0) drawPixel(i,c-1,0x0000);
			drawPixel(i,c,0x07E0);
		}
		if (c == 0){
			delay(1000);
		} else {
			delay(20);
		}
		
	}
}
*/
//not tested
void OLED_SSD1332::setBrightness(byte val){
	writeCommand(_CMD_MASTERCURRENT);
	if (val < 0x17) {
		writeData(val);
	} else {
		writeData(0x0F);
	}
}

/********************************* library */
OLED_SSD1332::OLED_SSD1332(uint8_t cs, uint8_t rs, uint8_t rst) : Adafruit_GFX(OLED_WIDTH, OLED_HEIGHT) {
    _cs = cs;
    _rs = rs;
    _rst = rst;
	_inited = false;
}

OLED_SSD1332::OLED_SSD1332(uint8_t cs, uint8_t rs) : Adafruit_GFX(OLED_WIDTH, OLED_HEIGHT) {
    _cs = cs;
    _rs = rs;
    _rst = 0;
	_inited = false;
}

//helper
void OLED_SSD1332::setRegister(const uint8_t reg,uint8_t val){
	uint8_t cmd[2] = {reg,val};
	writeCommands(cmd,2);
}

/********************************** low level pin and SPI transfer based on MCU */
#ifdef __AVR__

	inline void OLED_SSD1332::spiwrite(uint8_t c){
		SPDR = c;
		while(!(SPSR & _BV(SPIF)));
	}

	void OLED_SSD1332::writeCommand(uint8_t c){
		*rsport &= ~rspinmask;
		*csport &= ~cspinmask;
		spiwrite(c);
		*csport |= cspinmask;
	}

	void OLED_SSD1332::writeCommands(uint8_t *cmd, uint8_t length){
		*rsport &= ~rspinmask;
		*csport &= ~cspinmask;
		for (uint8_t i = 0; i < length; i++) {
			spiwrite(*cmd++);
		}
		*csport |= cspinmask;
	}
	
	void OLED_SSD1332::writeData(uint8_t c){
		*rsport |=  rspinmask;
		*csport &= ~cspinmask;
		spiwrite(c);
		*csport |= cspinmask;
	} 

	void OLED_SSD1332::setBitrate(uint32_t n){
		if (n >= 8000000) {
			SPI.setClockDivider(SPI_CLOCK_DIV2);
		} else if (n >= 4000000) {
			SPI.setClockDivider(SPI_CLOCK_DIV4);
		} else if (n >= 2000000) {
			SPI.setClockDivider(SPI_CLOCK_DIV8);
		} else {
			SPI.setClockDivider(SPI_CLOCK_DIV16);
		}
	}
#elif defined(__SAM3X8E__)


	inline void OLED_SSD1332::spiwrite(uint8_t c){
		SPI.transfer(c);
	}
	
	void OLED_SSD1332::writeCommand(uint8_t c){
		rsport->PIO_CODR |=  rspinmask;
		csport->PIO_CODR  |=  cspinmask;
		spiwrite(c);
		csport->PIO_SODR  |=  cspinmask;
	}
	
	void OLED_SSD1332::writeCommands(uint8_t *cmd, uint8_t length){
		rsport->PIO_CODR |=  rspinmask;
		csport->PIO_CODR  |=  cspinmask;
		for (uint8_t i = 0; i < length; i++) {
			spiwrite(*cmd++);
		}
		csport->PIO_SODR  |=  cspinmask;
	}
	
	void OLED_SSD1332::writeData(uint8_t c){
		rsport->PIO_SODR |=  rspinmask;
		csport->PIO_CODR  |=  cspinmask;
		spiwrite(c);
		csport->PIO_SODR  |=  cspinmask;
	} 
	
	
	void OLED_SSD1332::setBitrate(uint32_t n){
		uint32_t divider=1;
		while (divider < 255) {
			if (n >= 84000000 / divider) break;
			divider = divider - 1;
		}
		SPI.setClockDivider(divider);
	}
#elif defined(__MK20DX128__) || defined(__MK20DX256__)
	void OLED_SSD1332::writeCommand(uint8_t c){
		KINETISK_SPI0.PUSHR = c | (pcs_command << 16) | SPI_PUSHR_CTAS(0);
		while (((KINETISK_SPI0.SR) & (15 << 12)) > (3 << 12)) ; // wait if FIFO full
	}

	void OLED_SSD1332::writeCommands(uint8_t *cmd, uint8_t length){
		for (uint8_t i = 0; i < length; i++) {
			KINETISK_SPI0.PUSHR = *cmd++ | (pcs_command << 16) | SPI_PUSHR_CTAS(0);
			while (((KINETISK_SPI0.SR) & (15 << 12)) > (3 << 12)) ; // wait if FIFO full
		}
	}
	
	void OLED_SSD1332::writeData(uint8_t c){
		KINETISK_SPI0.PUSHR = c | (pcs_data << 16) | SPI_PUSHR_CTAS(0);
		while (((KINETISK_SPI0.SR) & (15 << 12)) > (3 << 12)) ; // wait if FIFO full
	}

	/*
	Helper:
	This function return true only if the choosed pin can be used for CS or RS
	*/
	static bool spi_pin_is_cs(uint8_t pin){
		if (pin == 2 || pin == 6 || pin == 9) return true;
		if (pin == 10 || pin == 15) return true;
		if (pin >= 20 && pin <= 23) return true;
		return false;
	}
	
	/*
	Helper:
	This function configure register in relation to pin
	*/
	static uint8_t spi_configure_cs_pin(uint8_t pin){
		switch (pin) {
			case 10: CORE_PIN10_CONFIG = PORT_PCR_MUX(2); return 0x01; // PTC4
			case 2:  CORE_PIN2_CONFIG  = PORT_PCR_MUX(2); return 0x01; // PTD0
			case 9:  CORE_PIN9_CONFIG  = PORT_PCR_MUX(2); return 0x02; // PTC3
			case 6:  CORE_PIN6_CONFIG  = PORT_PCR_MUX(2); return 0x02; // PTD4
			case 20: CORE_PIN20_CONFIG = PORT_PCR_MUX(2); return 0x04; // PTD5
			case 23: CORE_PIN23_CONFIG = PORT_PCR_MUX(2); return 0x04; // PTC2
			case 21: CORE_PIN21_CONFIG = PORT_PCR_MUX(2); return 0x08; // PTD6
			case 22: CORE_PIN22_CONFIG = PORT_PCR_MUX(2); return 0x08; // PTC1
			case 15: CORE_PIN15_CONFIG = PORT_PCR_MUX(2); return 0x10; // PTC0
		}
		return 0;
	}

	/*
	Helper:
	This function set the speed of the SPI interface
	*/
	void OLED_SSD1332::setBitrate(uint32_t n){
		if (n >= 24000000) {
			ctar = CTAR_24MHz;
		} else if (n >= 16000000) {
			ctar = CTAR_16MHz;
		} else if (n >= 12000000) {
			ctar = CTAR_12MHz;
		} else if (n >= 8000000) {
			ctar = CTAR_8MHz;
		} else if (n >= 6000000) {
			ctar = CTAR_6MHz;
		} else {
			ctar = CTAR_4MHz;
		}
		SIM_SCGC6 |= SIM_SCGC6_SPI0;
		KINETISK_SPI0.MCR = SPI_MCR_MDIS | SPI_MCR_HALT;
		KINETISK_SPI0.CTAR0 = ctar | SPI_CTAR_FMSZ(7);
		KINETISK_SPI0.CTAR1 = ctar | SPI_CTAR_FMSZ(15);
		KINETISK_SPI0.MCR = SPI_MCR_MSTR | SPI_MCR_PCSIS(0x1F) | SPI_MCR_CLR_TXF | SPI_MCR_CLR_RXF;
	}
#endif

/********************************** common helpers */

bool OLED_SSD1332::remapDirection(bool rev){
	if (rev){
		bitSet(_remapData,1);
		setRegister(_CMD_SETREMAP,_remapData);
	} else {
		bitClear(_remapData,1);
		setRegister(_CMD_SETREMAP,_remapData);////0b01110000
	}
	return rev;
}


bool OLED_SSD1332::fillTool(bool fillState){
	if (fillState){
		setRegister(_CMD_FILL,1);
	} else {
		setRegister(_CMD_FILL,0);
	}
	return fillState;
}



/********************************** special hardware functions */
/*
Hardware accellerated Draw Rect (filled or not)
*/
void OLED_SSD1332::hdwre_drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color, bool filled){
    #if defined(__MDBG)
	Serial.println();
	Serial.print("oX:");
	Serial.print(x,DEC);
	Serial.print(" oY:");
	Serial.print(y,DEC);
	Serial.print(" oW:");
	Serial.print(w,DEC);
	Serial.print(" oH:");
	Serial.print(h,DEC);
	Serial.print(" - ");
	#endif
	//rotation??
	switch (getRotation()) {
	case 0:						//UpsideDown
	case 2:
		//if ((y+h) > HEIGHT) h = HEIGHT - y;
		//if ((x+w) > WIDTH) w = WIDTH - x;						
		break;
	case 1:
		//swap(x, y);
		swap(w, h);
		//x = WIDTH - x - 1;
		//x = width() - x - 1;
		#if defined(__MDBG)
		Serial.print(" nX:");
		Serial.print(x,DEC);
		Serial.print(" nY:");
		Serial.print(y,DEC);
		Serial.print(" nW:");
		Serial.print(w,DEC);
		Serial.print(" nH:");
		Serial.print(h,DEC);
		Serial.print(" -> ");
		#endif
		break;
	//case 2:						//normal
		//x = WIDTH - x - 1;
		//y = HEIGHT - y - 1;
		//x = width() - x - 1;
		//y = height() - y - 1;
		// if ((y+h) > HEIGHT) h = HEIGHT - y;
		// if ((x+w) > WIDTH) w = WIDTH - x;
		// #if defined(__MDBG)
		// Serial.print(" nX:");
		// Serial.print(x,DEC);
		// Serial.print(" nY:");
		// Serial.print(y,DEC);
		// Serial.print(" nW:");
		// Serial.print(w,DEC);
		// Serial.print(" nH:");
		// Serial.print(h,DEC);
		// Serial.print(" -> ");
		// #endif
		// break;
	case 3:
		swap(x, y);
		swap(w, h);
		//y = HEIGHT - y - 1;
		//y = height() - y - 1;
		#if defined(__MDBG)
		Serial.print(" nX:");
		Serial.print(x,DEC);
		Serial.print(" nY:");
		Serial.print(y,DEC);
		Serial.print(" nW:");
		Serial.print(w,DEC);
		Serial.print(" nH:");
		Serial.print(h,DEC);
		Serial.print(" -> ");
		#endif
		break;
	}
	/*
  switch(rotation) {
   case 0:

		
		if ((x > WIDTH)) x = WIDTH;
		if ((y > WIDTH)) y = HEIGHT;
		if ((w > WIDTH)) w = WIDTH;
		if ((h > HEIGHT)) h = HEIGHT;
		break;
   case 1:

		break;
   case 2:

		break;
   case 3:
		if ((y+h) > HEIGHT) h = HEIGHT - y;
		if ((x+w) > WIDTH) w = WIDTH - x;
		break;
  }
	*/
	// Bounds check
	//if ((x >= WIDTH) || (y >= HEIGHT)) return;
	//if (x >= width() || y >= height()) return;
	if ((y+h) > HEIGHT) h = HEIGHT - y;
	if ((x+w) > WIDTH) w = WIDTH - x;
	#if defined(__MDBG)
	Serial.print(" SWidth:");
	Serial.print(WIDTH,DEC);
	Serial.print(" SHeight:");
	Serial.print(HEIGHT,DEC);
	Serial.print(" fX:");
	Serial.print(x,DEC);
	Serial.print(" fY:");
	Serial.print(y,DEC);
	Serial.print(" fW:");
	Serial.print(w,DEC);
	Serial.print(" fH:");
	Serial.print(h,DEC);
	Serial.println();
	#endif
	
    //  to fill or not to fill?
    if (filled) {
        if (!_fillEnabled) _fillEnabled = fillTool(true);
    } else {
        if (_fillEnabled) _fillEnabled = fillTool(false);
    }

    if (_remap) _remap = remapDirection(false);
	uint8_t c1 = (color & 0xF800) >> 10, c2 = (color & 0x07E0) >> 5, c3 = (color & 0x001F) << 1;
    //  draw/fill the rectangle
	uint8_t cmds[11];//container
	cmds[0] = _CMD_DRAWRECT;
	cmds[1] = x; //x1
	cmds[2] = y; //y1
	cmds[3] = (x + w) - 1; //x2
	cmds[4] = (y + h) - 1; //y2
	cmds[5] = c1; cmds[6] = c2; cmds[7]  = c3; //color h
	cmds[8] = c1; cmds[9] = c2; cmds[10] = c3; //color l
	writeCommands(cmds, 11);
	delayMicroseconds(_DLY_FILL);
}

/*
----------------------------------------- HI SPEED SPI
*/

/*
Initialize PIN, direction and stuff related to hardware on CPU
*/
void OLED_SSD1332::commonInit(){
#if defined(__AVR__) 
	pinMode(_rs, OUTPUT);
	pinMode(_cs, OUTPUT);
	csport    = portOutputRegister(digitalPinToPort(_cs));
	rsport    = portOutputRegister(digitalPinToPort(_rs));
	cspinmask = digitalPinToBitMask(_cs);
	rspinmask = digitalPinToBitMask(_rs);
    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV4); // 4 MHz (half speed)
    //Due defaults to 4mHz (clock divider setting of 21)
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
	*csport &= ~cspinmask;
	_inited = true;
#elif defined(__SAM3X8E__) 
	pinMode(_rs, OUTPUT);
	pinMode(_cs, OUTPUT);
	csport    = digitalPinToPort(_cs);
	rsport    = digitalPinToPort(_rs);
	cspinmask = digitalPinToBitMask(_cs);
	rspinmask = digitalPinToBitMask(_rs);
    SPI.begin();
    SPI.setClockDivider(21); // 4 MHz
    //Due defaults to 4mHz (clock divider setting of 21), but we'll set it anyway 
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
	// toggle RST low to reset; CS low so it'll listen to us
	csport ->PIO_CODR  |=  cspinmask; // Set control bits to LOW (idle)
	_inited = true;
#elif defined(__MK20DX128__) || defined(__MK20DX256__)
	_sid = 11;//here you can select witch SPI interface to use
	_sclk = 13;//here you can select witch SPI interface to use
	if (spi_pin_is_cs(_cs) && spi_pin_is_cs(_rs)
	 && (_sid == 7 || _sid == 11) && (_sclk == 13 || _sclk == 14)
	 && !(_cs ==  2 && _rs == 10) && !(_rs ==  2 && _cs == 10)
	 && !(_cs ==  6 && _rs ==  9) && !(_rs ==  6 && _cs ==  9)
	 && !(_cs == 20 && _rs == 23) && !(_rs == 20 && _cs == 23)
	 && !(_cs == 21 && _rs == 22) && !(_rs == 21 && _cs == 22)) {
		if (_sclk == 13) {
			CORE_PIN13_CONFIG = PORT_PCR_MUX(2) | PORT_PCR_DSE;
			SPCR.setSCK(13);
		} else {
			CORE_PIN14_CONFIG = PORT_PCR_MUX(2);
			SPCR.setSCK(14);
		}
		if (_sid == 11) {
			CORE_PIN11_CONFIG = PORT_PCR_MUX(2);
			SPCR.setMOSI(11);
		} else {
			CORE_PIN7_CONFIG = PORT_PCR_MUX(2);
			SPCR.setMOSI(7);
		}
		ctar = CTAR_12MHz;
		pcs_data = spi_configure_cs_pin(_cs);
		pcs_command = pcs_data | spi_configure_cs_pin(_rs);
		SIM_SCGC6 |= SIM_SCGC6_SPI0;
		KINETISK_SPI0.MCR = SPI_MCR_MDIS | SPI_MCR_HALT;
		KINETISK_SPI0.CTAR0 = ctar | SPI_CTAR_FMSZ(7);
		KINETISK_SPI0.CTAR1 = ctar | SPI_CTAR_FMSZ(15);
		KINETISK_SPI0.MCR = SPI_MCR_MSTR | SPI_MCR_PCSIS(0x1F) | SPI_MCR_CLR_TXF | SPI_MCR_CLR_RXF;
		_inited = true;
	} else {
		_inited = false;
		//error! cannot continue
		// TODO!  Escape code to stop all
	}	
#endif
	if (_inited && _rst) {
		pinMode(_rst, OUTPUT);
		digitalWrite(_rst, HIGH);
		delay(500);
		digitalWrite(_rst, LOW);
		delay(500);
		digitalWrite(_rst, HIGH);
		delay(500);
	}
	_remapData = 0b01100010;
	/*
	0)h adrs increment
	1)95 mapped to 0
	2)x
	3)x
	4)scan 0 to n-1
	5)enable com split
	6,7)65Kcolors
	*/
}

/*
Here's the
*/
void OLED_SSD1332::chipInit() {
	
	//set SSD chip registers
	uint8_t cmd[33];
	writeCommand(_CMD_DISPLAYOFF);  	
	/*
	0) - 0:H address increment / 1:V address increment
	1) - 0:0 mapped to 0 / 1:95 mapped to 0
	2) - na
	3) - na
	4) - 0:scan from com 0 to com n-1 / 1:scan from com n-1 to com 0
	5) - 0:disable com split / 1:enable com split
	6-7) - 00:256 colors / 01:65K colors
	*/
	writeCommand(_CMD_SETREMAP); 	
	writeCommand(_remapData);////0b01100010
	setRegister(_CMD_FILL,0x01);
	setRegister(_CMD_STARTLINE,0x00);//default 0x00	
	setRegister(_CMD_DISPLAYOFFSET,0x00);//default 0x00
	setRegister(_CMD_PHASEPERIOD,0b10110001);
	setRegister(_CMD_SETMULTIPLEX,0x3F);
	setRegister(_CMD_SETMASTER,0x8E);
	setRegister(_CMD_POWERMODE,0b00001011);
	setRegister(_CMD_PRECHARGE,0x1F);//0x1F - 0x31
	setRegister(_CMD_CLOCKDIV,0xF0);
	//setRegister(_CMD_PRECHARGEA,0x64);
	//setRegister(_CMD_PRECHARGEB,0x78);
	//setRegister(_CMD_PRECHARGEC,0x64);
	setRegister(_CMD_PRECHARGELEVEL,0x3A);//0x3A - 0x00
	setRegister(_CMD_VCOMH,0x3F);//0x3E - 0x3F
	setRegister(_CMD_MASTERCURRENT,0x0F);//0x06 - 0x0F
	//setRegister(_CMD_CONTRASTA,0x91);//0xEF - 0x91
	//setRegister(_CMD_CONTRASTB,0x50);//0x11 - 0x50
	//setRegister(_CMD_CONTRASTC,0x7D);//0x48 - 0x7D
	//setRegister(_CMD_VPACOLORLVL,0x40);
	//setRegister(_CMD_VPBCOLORLVL,0x40);
	//setRegister(_CMD_VPCCOLORLVL,0x40);
	cmd[0] = _CMD_GRAYSCALE;
	cmd[1] =  0x01; cmd[2] =  0x03; cmd[3] =  0x05; cmd[4] =  0x07; cmd[5] =  0x0A; cmd[6] =  0x0D; cmd[7] =  0x10; cmd[8] =  0x13;
	cmd[9] =  0x16; cmd[10] = 0x19; cmd[11] = 0x1C; cmd[12] = 0x20; cmd[13] = 0x24; cmd[14] = 0x28; cmd[15] = 0x2C; cmd[16] = 0x30;
	cmd[17] = 0x34; cmd[18] = 0x38; cmd[19] = 0x3C; cmd[20] = 0x40; cmd[21] = 0x44; cmd[22] = 0x48; cmd[23] = 0x4C; cmd[24] = 0x50;
	cmd[25] = 0x54; cmd[26] = 0x58; cmd[27] = 0x5C; cmd[28] = 0x60; cmd[29] = 0x64; cmd[30] = 0x68; cmd[31] = 0x6C; cmd[32] = 0x70;
	writeCommands(cmd, 33);
	writeCommand(_CMD_NORMALDISPLAY);
	writeCommand(_CMD_DISPLAYON);
	_fillEnabled = true;
	_remap = false;
	clearScreen(0x0000);

}
