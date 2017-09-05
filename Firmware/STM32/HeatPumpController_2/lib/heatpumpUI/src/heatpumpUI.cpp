#include "heatpumpUI.h"


DebouncedInput buttonUP(28, 10, true, 1); 
DebouncedInput buttonDown(29, 10, true, 1); 
DebouncedInput buttonSelect(30, 10, true, 1); 
DebouncedInput buttonOnOff(31, 10, true, 1); 
CountDown CD(CountDown::SECONDS);

#define DELAY_TIME_SET_CONFIG 6

heatpumpUI::heatpumpUI() : selectedMenu(MENU_SELECTED::NONE)
{
    redraw = true;
    editingTemp = false;
    runningConfig = currentConfig;
}

void heatpumpUI::begin(U8G2_ST7920_128X64_F_SW_SPI *display)
{
    //u8g2.begin();
    display_ = display;
}

void heatpumpUI::DrawMainScreen()
{
    if(!redraw)
    {
        return;
    }

    display_->clearBuffer();
    DrawCurrentTemperature();
    DrawCompOn_Off();
    DrawFanOn_Off();
    DrawConfigTemperature();
    DrawAROn_Off();
    DrawRoomConfig();    
    DrawFanSpeedConfig();
    DrawModeConfig();
    DrawFanModeConfig();
    DrawGrid();
    display_->sendBuffer();
    redraw = false;
}

void heatpumpUI::DrawCurrentTemperature()
{
#define CURTEMP_X   7
#define CURTEMP_Y  30   
    //Formating current temperature
    int currentTemp = (int)currentStatus.temperature;
    int currentTempDecimal = 10000 * (currentStatus.temperature - currentTemp + 0.00005);

    //Draw current temp
    display_->setFont(FONT_MAIN_TEMP);
    String temperatureStr = String(currentTemp);
    if (currentTemp < 10)
    {
        temperatureStr = String(" " + temperatureStr);
    }
    display_->drawStr(0, 30, temperatureStr.c_str());

    //Draw decimal current temp
    display_->setFont(FONT_DECIMAL_TEMP);
    String decimaTemperatureStr = String(currentTempDecimal).substring(0, 1);
    display_->drawStr(CURTEMP_X + 37, CURTEMP_Y, decimaTemperatureStr.c_str());

    //Dray C symbol
    display_->setFont(FONT_DEGREE_SYMBOL);
    display_->drawGlyph(CURTEMP_X + 37, CURTEMP_Y - 17, 176);
    display_->drawStr(CURTEMP_X + 42, CURTEMP_Y - 17, "C");
}

void heatpumpUI::DrawConfigTemperature()
{
    display_->setFont(FONT_GENERAL_10PIXEL);
    if( selectedMenu == MENU_SELECTED(MENU_SELECTED::TEMPERATURE))
    {
        display_->setDrawColor(2);
        display_->setFontMode(1);
        display_->drawBox(30, 31, 28, 17);
    }
    //Draw config temp
    
    display_->drawStr(32, 45, String(currentConfig.temperature, 1).c_str());
    display_->setDrawColor(1);
    display_->setFontMode(1);
}
void heatpumpUI::DrawCompOn_Off()
{
    display_->setFont(FONT_GENERAL_10PIXEL);
    //Draw Compressor ON/OFF
    if( currentStatus.compressorON )
    {
        display_->setDrawColor(2);
        display_->setFontMode(1);
        display_->drawBox(0, 47, 29, 17);
    }
    display_->drawStr(8, 61, "C");
    display_->setDrawColor(1);
    display_->setFontMode(1);
}
void heatpumpUI::DrawFanOn_Off()
{
    display_->setFont(FONT_GENERAL_10PIXEL);
    //Draw FAN ON/OFF
    if( currentStatus.fanOn )
    {
        display_->setDrawColor(2);
        display_->setFontMode(1);
        display_->drawBox(30, 47, 29, 17);
    }
    display_->drawStr(40, 61, "F");
    display_->setDrawColor(1);
    display_->setFontMode(1);
}

void heatpumpUI::DrawAROn_Off()
{
    display_->setFont(FONT_GENERAL_10PIXEL);
    //Draw AR ON/OFF
    if( currentConfig.arconON )
    {
        display_->setDrawColor(2);
        display_->setFontMode(1);
        display_->drawBox(0, 31, 29, 17);
    }
    display_->drawStr(5, 45, "AR");
    display_->setDrawColor(1);
    display_->setFontMode(1);
}

void heatpumpUI::DrawFanModeConfig()
{
    display_->setFont(FONT_GENERAL_10PIXEL);

    if( selectedMenu == MENU_SELECTED(MENU_SELECTED::FAN))
    {
        display_->setDrawColor(2);
        display_->setFontMode(1);
        display_->drawBox(60, 0, 21, 16);
    }
    display_->drawStr(60, 13, "FM");
    display_->setDrawColor(1);
    display_->setFontMode(1);

    if( currentConfig.fan_mode == FAN_MODE(FAN_MODE::AUTO))
    {
        display_->setDrawColor(2);
        display_->setFontMode(1);
        display_->drawBox(83, 1, 22, 14);
    }
    display_->drawStr(84, 13, "AU");
    display_->setDrawColor(1);
    display_->setFontMode(1);

    if( currentConfig.fan_mode == FAN_MODE(FAN_MODE::CONT))
    {
        display_->setDrawColor(2);
        display_->setFontMode(1);
        display_->drawBox(105, 1, 22, 14);
    }
    display_->drawStr(105, 13, "CO");
    display_->setDrawColor(1);
    display_->setFontMode(1);
}

void heatpumpUI::DrawFanSpeedConfig()
{
    display_->setFont(FONT_GENERAL_10PIXEL);
    if( selectedMenu == MENU_SELECTED(MENU_SELECTED::FAN_SPEED))
    {
        display_->setDrawColor(2);
        display_->setFontMode(1);
        display_->drawBox(60, 17, 21, 16);
    }
    display_->drawStr(62, 29, "FS");
    display_->setDrawColor(1);
    display_->setFontMode(1);

    if( currentConfig.fan_speed == FAN_SPEED(FAN_SPEED::SPEED_1 ))
    {
        display_->setDrawColor(2);
        display_->setFontMode(1);
        display_->drawBox(83, 18, 10, 13);
    }
    display_->drawStr(84, 29, "1");
    display_->setDrawColor(1);
    display_->setFontMode(1);

    if( currentConfig.fan_speed == FAN_SPEED(FAN_SPEED::SPEED_2 ))
    {
        display_->setDrawColor(2);
        display_->setFontMode(1);
        display_->drawBox(99, 18, 10, 13);
    }
    display_->drawStr(100, 29, "2");
    display_->setDrawColor(1);
    display_->setFontMode(1);

    if( currentConfig.fan_speed == FAN_SPEED(FAN_SPEED::SPEED_3 ))
    {
        display_->setDrawColor(2);
        display_->setFontMode(1);
        display_->drawBox(115, 18, 10, 13);
    }
    display_->drawStr(116, 29, "3");
    display_->setDrawColor(1);
    display_->setFontMode(1);
}

void heatpumpUI::DrawModeConfig()
{
    display_->setFont(FONT_GENERAL_10PIXEL);

    if( selectedMenu == MENU_SELECTED(MENU_SELECTED::COLD_HOT))
    {
        display_->setDrawColor(2);
        display_->setFontMode(1);
        display_->drawBox(60, 33, 21, 16);
    }
    display_->drawStr(59, 45, "MD");
    display_->setDrawColor(1);
    display_->setFontMode(1);

    if( currentConfig.mode == PUMP_MODE(PUMP_MODE::COLD))
    {
        display_->setDrawColor(2);
        display_->setFontMode(1);
        display_->drawBox(83, 34, 22, 13);
    }
    display_->drawStr(83, 45, "CD");
    display_->setDrawColor(1);
    display_->setFontMode(1);

    if( currentConfig.mode == PUMP_MODE(PUMP_MODE::HOT))
    {
        display_->setDrawColor(2);
        display_->setFontMode(1);
        display_->drawBox(105, 34, 23, 13);
    }
    display_->drawStr(105, 45, "HT");
    display_->setDrawColor(1);
    display_->setFontMode(1);
}

void heatpumpUI::DrawRoomConfig()
{
    display_->setFont(FONT_GENERAL_10PIXEL);
    if( selectedMenu == MENU_SELECTED(MENU_SELECTED::ROOM))
    {
        display_->setDrawColor(2);
        display_->setFontMode(1);
        display_->drawBox(60, 49, 21, 16);
    }
    display_->drawStr(65, 61, "R");
    display_->setDrawColor(1);
    display_->setFontMode(1);

    if( currentConfig.room == CONFIGURED_ROOM(CONFIGURED_ROOM::BEDROOM ))
    {
        display_->setDrawColor(2);
        display_->setFontMode(1);
        display_->drawBox(83, 50, 13, 14);
    }
    display_->drawStr(84, 61, "Q");
    display_->setDrawColor(1);
    display_->setFontMode(1);

    if( currentConfig.room == CONFIGURED_ROOM(CONFIGURED_ROOM::LIVING_ROOM ))
    {
        display_->setDrawColor(2);
        display_->setFontMode(1);
        display_->drawBox(99, 50, 10, 14);
    }
    display_->drawStr(100, 61, "S");
    display_->setDrawColor(1);
    display_->setFontMode(1);

    if( currentConfig.room == CONFIGURED_ROOM(CONFIGURED_ROOM::OFFICE ))
    {
        display_->setDrawColor(2);
        display_->setFontMode(1);
        display_->drawBox(115, 50, 11, 14);
    }
    display_->drawStr(116, 61, "E");
    display_->setDrawColor(1);
    display_->setFontMode(1);
}

void heatpumpUI::DrawGrid()
{
#define  D_SIZE_X 128
#define  D_SIZE_Y 64

#define  B1_X 58
#define  B1_Y 0
#define  B1_LENGTH D_SIZE_Y

#define  B2_X 0
#define  B2_Y 31
#define  B2_LENGTH B1_X

#define  B3_X B1_X/2
#define  B3_Y B2_Y
#define  B3_LENGTH  (D_SIZE_Y - B3_Y)

#define  B4_X 0
#define  B4_Y ((D_SIZE_Y - B2_Y)/2) + B2_Y
#define  B4_LENGTH  B1_X

    //                      B1 
    //    -------------------------------------------------
    //    |                  |                            |  
    //    |   curTemp        |                            |    
    //    |                  |                            |    
    //    |        B3        |                            |    
    //  B2|-------------------                            |    
    //    |  ON/OFF | Config |                            |    
    //    |         |  Temp  |                            |    
    //  B4|------------------|                            |    
    //    | Comp    | Vent   |                            |    
    //    |   On    |   ON   |                            |    
    //    -------------------------------------------------

//B1
    display_->drawVLine(B1_X, B1_Y, B1_LENGTH);
    display_->drawVLine(B1_X + 1, B1_Y, B1_LENGTH);
//B2
    display_->drawHLine(B2_X, B2_Y, B2_LENGTH);
    display_->drawHLine(B2_X, B2_Y + 1, B2_LENGTH);
//B3
    display_->drawVLine(B3_X, B3_Y, B3_LENGTH);
//B4
    display_->drawHLine(B4_X, B4_Y, B4_LENGTH);


#define  B5_X B1_X
#define  B5_Y ((D_SIZE_Y/4)*1)
#define  B5_LENGTH  D_SIZE_X - B1_X

#define  B6_X B1_X
#define  B6_Y ((D_SIZE_Y/4)*2)
#define  B6_LENGTH  D_SIZE_X - B1_X

#define  B7_X B1_X
#define  B7_Y ((D_SIZE_Y/4)*3)
#define  B7_LENGTH  D_SIZE_X - B1_X

#define  B8_X B1_X + 23
#define  B8_Y 0
#define  B8_LENGTH  D_SIZE_Y
    //                       B1      B8
    //    -------------------------------------------------
    //    |                  |       |                    |  
    //    |                  |       |                    |    
    //    |                  |----------------------------| B5    
    //    |                  |       |                    |    
    //    |                  |       |                    |    
    //    |                  |-------=--------------------| B6   
    //    |                  |       |                    |    
    //    |                  |       |                    |    
    //    |                  |----------------------------| B7   
    //    |                  |       |                    |    
    //    -------------------------------------------------
//B5
    display_->drawHLine(B5_X, B5_Y, B5_LENGTH);
//B6
    display_->drawHLine(B6_X, B6_Y, B6_LENGTH);
//B7
    display_->drawHLine(B7_X, B7_Y, B7_LENGTH);
//B8
    display_->drawVLine(B8_X, B8_Y, B8_LENGTH);
}

void heatpumpUI::resetPressedButtons()
{
    pressedButtons.on_off = false;
    pressedButtons.up = false;
    pressedButtons.down = false;
    pressedButtons.select = false;
}

void heatpumpUI::HandleButtons()
{
    if(buttonUP.changedTo(HIGH))
    {
        pressedButtons.up = true;
    }
    if(buttonDown.changedTo(HIGH))
    {
        pressedButtons.down = true;
    }
    if(buttonSelect.changedTo(HIGH))
    {
        pressedButtons.select = true;
    }
    if(buttonOnOff.changedTo(HIGH))
    {
        pressedButtons.on_off = true;
    }    
}

void heatpumpUI::HandleActions()
{
    if(editingTemp)
    {
        if(pressedButtons.up)
        {
            if( currentConfig.temperature < 30){
                currentConfig.temperature += 0.5;
            }
            redraw = true;
        }

        if(pressedButtons.down)
        {
            if( currentConfig.temperature > 10){
                currentConfig.temperature -= 0.5;
            }
            redraw = true;            redraw = true;
        }
    }
    else
    {
        if(pressedButtons.up)
        {
            selectedMenu = static_cast<MENU_SELECTED>( static_cast<int>(selectedMenu) + 1);
            if(selectedMenu == MENU_SELECTED::LAST)
            {
                selectedMenu = MENU_SELECTED::NONE;
            }
            redraw = true;
        }

        if(pressedButtons.down)
        {
            if(selectedMenu == MENU_SELECTED::NONE)
            {
                selectedMenu = MENU_SELECTED::ROOM;
            }
            else{
                selectedMenu = static_cast<MENU_SELECTED>( static_cast<int>(selectedMenu) - 1);
            }
            redraw = true;
        }
    }

    if(pressedButtons.on_off)
    {
        currentConfig.arconON = !currentConfig.arconON;
        redraw = true;
    }

    if(pressedButtons.select)
    {
        switch(selectedMenu){
            case MENU_SELECTED::TEMPERATURE:
                editingTemp = !editingTemp;
                redraw = true;
                break;
            case MENU_SELECTED::FAN:
                if(currentConfig.fan_mode == FAN_MODE::AUTO)
                {
                    currentConfig.fan_mode = FAN_MODE::CONT;
                }
                else
                {
                    currentConfig.fan_mode = FAN_MODE::AUTO;
                }
                redraw = true;
                break;
            case MENU_SELECTED::FAN_SPEED:
                if(currentConfig.fan_speed == FAN_SPEED::SPPED_3)
                {
                    currentConfig.fan_speed = FAN_SPEED::SPPED_1;
                }
                else
                {
                    currentConfig.fan_speed = static_cast<FAN_SPEED>( static_cast<int>(currentConfig.fan_speed) + 1);
                }
                redraw = true;
                break;
            case MENU_SELECTED::COLD_HOT:
                if(currentConfig.mode == PUMP_MODE::HOT)
                {
                    currentConfig.mode = PUMP_MODE::COLD;
                }
                else
                {
                    currentConfig.mode = PUMP_MODE::HOT;
                }
                redraw = true;
                break;
            case MENU_SELECTED::ROOM:
                if(currentConfig.room == CONFIGURED_ROOM::OFFICE)
                {
                    currentConfig.room = CONFIGURED_ROOM::BEDROOM;
                }
                else{
                    currentConfig.room = static_cast<CONFIGURED_ROOM>( static_cast<int>(currentConfig.room) + 1);
                }
                redraw = true;
                break;
            default:
                break;
        }
    }

    if(currentConfig == runningConfig)
    {
        if(CD.isRunning())
        {
            CD.stop();
        }
    }
    else
    {   
        if(!(lastReeadConfig == currentConfig))
        {
            CD.start(DELAY_TIME_SET_CONFIG);
        }
        if(CD.isRunning())
        {
            if(CD.remaining() == 0)
            {
                runningConfig = currentConfig;
                CD.stop();
            }
        }else
        {
            CD.start(DELAY_TIME_SET_CONFIG);
        }
    }
    lastReeadConfig = currentConfig;
}

HeatPumpConfig heatpumpUI::getConfig()
{
    return runningConfig;
}

void heatpumpUI::run()
{   
    HandleButtons();
    HandleActions();
    DrawMainScreen();
    resetPressedButtons();
}
