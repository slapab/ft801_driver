#include "demo_tasks.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ft80x_it_api_cmd.h"
#include "ft801_gpu.h"


bool demoMenuTask_painting (void * const data)
{
    ft801_api_cmd_prepare_it(FT_RAM_CMD) ;
    ft801_api_cmd_append_it(CMD_DLSTART) ;
    
    ft801_api_cmd_append_it(CLEAR_COLOR_RGB(0,0,0));
    ft801_api_cmd_append_it(CLEAR(1,1,1));
    
    
    
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(COLOR_RGB(255,255,255));
    
    ft801_api_cmd_text_it(420,23,31, 1536, "18:20");
    
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(COLOR_RGB(255,255,255));
    
    ft801_api_cmd_fgcolor_it(0x0080ff);
    ft801_api_cmd_gradcolor_it(0xffffff);
    
    ft801_api_cmd_button_it(14,12, 139,40, 28,256, "Ustawienia");
    
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(COLOR_RGB(255,255,255));
    ft801_api_cmd_fgcolor_it(0x0080ff);
    ft801_api_cmd_gradcolor_it(0xffffff);
    
    ft801_api_cmd_button_it(14,65, 139,40, 28,256, "Wykres");
    
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(COLOR_RGB(0,128,255));
    ft801_api_cmd_append_it(SCISSOR_XY(157,189));
    ft801_api_cmd_append_it(SCISSOR_SIZE(322,79));
    ft801_api_cmd_append_it(BEGIN(FT_EDGE_STRIP_B));
    ft801_api_cmd_append_it(VERTEX2F(2512,4272));
    ft801_api_cmd_append_it(VERTEX2F(3040,3024));
    ft801_api_cmd_append_it(VERTEX2F(7648,3024));
    ft801_api_cmd_append_it(END());
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(COLOR_RGB(255,255,255));
    
    ft801_api_cmd_text_it(203,182,26, 1536, "Logi");

    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(POINT_SIZE(224));
    ft801_api_cmd_append_it(COLOR_RGB(0,128,255));
    ft801_api_cmd_append_it(BEGIN(FT_POINTS));
    ft801_api_cmd_append_it(VERTEX2F(2784,3648));
    ft801_api_cmd_append_it(END());
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(POINT_SIZE(144));
    ft801_api_cmd_append_it(COLOR_RGB(255,255,255));
    ft801_api_cmd_append_it(BEGIN(FT_POINTS));
    ft801_api_cmd_append_it(VERTEX2F(2800,3648));
    ft801_api_cmd_append_it(END());
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(COLOR_RGB(255,0,0));
    
    ft801_api_cmd_text_it(334,203,16, 1536, "12:30 [warn] krtyczna wartosc mocy");
    
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(COLOR_RGB(255,255,255));
    
    ft801_api_cmd_text_it(317,221,16, 1536, "12:30 [info] prad lasera 10 mA");
    
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(COLOR_RGB(255,255,255));
    
    ft801_api_cmd_text_it(328,241,16, 1536, "12:31 [info] poziom mocy w normie");
    
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(COLOR_RGB(255,85,0));
    ft801_api_cmd_bgcolor_it(0x0065ca);
    ft801_api_cmd_progress_it(304,83,162,9, 256,1,2);
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(COLOR_RGB(255,255,255));
    
    ft801_api_cmd_text_it(318,72,28, 1536, "56 C");
    
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    
    
    ft801_api_cmd_append_it(DISPLAY());
    ft801_api_cmd_append_it(CMD_SWAP);
    ft801_api_cmd_flush_it() ;
    
    
    return true;
}

bool demoMenuTask_doing(void * const data)
{
    return true ;
}
bool demoMenuTask_gpuit(const uint8_t itflags, void * const data)
{
    return true;
}




bool demoGraphTask_painting(void * const data)
{
    ft801_api_cmd_prepare_it(FT_RAM_CMD) ;
    ft801_api_cmd_append_it(CMD_DLSTART) ;
    
    ft801_api_cmd_append_it(CLEAR_COLOR_RGB(0,0,0));
    ft801_api_cmd_append_it(CLEAR(1,1,1));
    
    
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(COLOR_RGB(126,126,126));
    ft801_api_cmd_append_it(COLOR_A(80) );
    ft801_api_cmd_append_it(LINE_WIDTH(16));
    ft801_api_cmd_append_it(BEGIN(FT_RECTS));
    ft801_api_cmd_append_it(VERTEX2F(560,128));
    ft801_api_cmd_append_it(VERTEX2F(7488,3872));
    ft801_api_cmd_append_it(END());
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    
    
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(LINE_WIDTH(48));
    ft801_api_cmd_append_it(COLOR_RGB(0,85,255));
    ft801_api_cmd_append_it(BEGIN(FT_LINES));
    ft801_api_cmd_append_it(VERTEX2F(592,160));
    ft801_api_cmd_append_it(VERTEX2F(592,3856));
    ft801_api_cmd_append_it(END());
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(LINE_WIDTH(48));
    ft801_api_cmd_append_it(COLOR_RGB(0,85,255));
    ft801_api_cmd_append_it(BEGIN(FT_LINES));
    ft801_api_cmd_append_it(VERTEX2F(7472,3872));
    ft801_api_cmd_append_it(VERTEX2F(592,3872));
    ft801_api_cmd_append_it(END());
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(COLOR_RGB(0,85,255));
    ft801_api_cmd_text_it(458,264,27, 1536, "12:50");
    
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(COLOR_RGB(255,255,255));
    
    ft801_api_cmd_text_it(92,255,26, 1536, "1ms");
    ft801_api_cmd_text_it(144,255,26, 1536, "2ms");    
    ft801_api_cmd_text_it(196,255,26, 1536, "3ms");
    ft801_api_cmd_text_it(251,255,26, 1536, "4ms");
    ft801_api_cmd_text_it(305,255,26, 1536, "5ms");
    ft801_api_cmd_text_it(359,255,26, 1536, "7ms");
    ft801_api_cmd_text_it(413,255,26, 1536, "8ms");
    
    ft801_api_cmd_append_it(RESTORE_CONTEXT());

    ft801_api_cmd_append_it(SAVE_CONTEXT());
    
    ft801_api_cmd_append_it(COLOR_RGB(0,128,255));
    ft801_api_cmd_append_it(COLOR_A(30) );
    ft801_api_cmd_append_it(LINE_WIDTH(16));
    ft801_api_cmd_append_it(BEGIN(FT_RECTS));
    ft801_api_cmd_append_it(VERTEX2F(672,128));
    ft801_api_cmd_append_it(VERTEX2F(1424,3824));
    ft801_api_cmd_append_it(END());

//    ft801_api_cmd_append_it(COLOR_RGB(0,128,255));
//    ft801_api_cmd_append_it(LINE_WIDTH(16));
    ft801_api_cmd_append_it(BEGIN(FT_RECTS));
    ft801_api_cmd_append_it(VERTEX2F(2304,128));
    ft801_api_cmd_append_it(VERTEX2F(3152,3824));
    ft801_api_cmd_append_it(END());

//    ft801_api_cmd_append_it(COLOR_RGB(0,128,255));
//    ft801_api_cmd_append_it(LINE_WIDTH(16));
    ft801_api_cmd_append_it(BEGIN(FT_RECTS));
    ft801_api_cmd_append_it(VERTEX2F(4032,128));
    ft801_api_cmd_append_it(VERTEX2F(4880,3824));
    ft801_api_cmd_append_it(END());

//    ft801_api_cmd_append_it(COLOR_RGB(0,128,255));
//    ft801_api_cmd_append_it(LINE_WIDTH(16));
    ft801_api_cmd_append_it(BEGIN(FT_RECTS));
    ft801_api_cmd_append_it(VERTEX2F(5616,128));
    ft801_api_cmd_append_it(VERTEX2F(6464,3824));
    ft801_api_cmd_append_it(END());

//    ft801_api_cmd_append_it(COLOR_RGB(0,128,255));
//    ft801_api_cmd_append_it(LINE_WIDTH(16));
    ft801_api_cmd_append_it(BEGIN(FT_RECTS));
    ft801_api_cmd_append_it(VERTEX2F(7344,128));
    ft801_api_cmd_append_it(VERTEX2F(7504,3824));
    ft801_api_cmd_append_it(END());

    
    ft801_api_cmd_append_it(COLOR_A(255) );
    ft801_api_cmd_append_it(LINE_WIDTH(16));
    ft801_api_cmd_append_it(COLOR_RGB(255,255,255));
    ft801_api_cmd_append_it(BEGIN(FT_LINES));
    ft801_api_cmd_append_it(VERTEX2F(624,1760));
    ft801_api_cmd_append_it(VERTEX2F(7504,1760));
    
    ft801_api_cmd_append_it(END());
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(COLOR_RGB(255,255,255));
    
    ft801_api_cmd_text_it(26,103,26, 1536, "60 mW");
    
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(LINE_WIDTH(16));
    ft801_api_cmd_append_it(COLOR_RGB(255,0,0));
    ft801_api_cmd_append_it(BEGIN(FT_LINES));
    ft801_api_cmd_append_it(VERTEX2F(624,800));
    ft801_api_cmd_append_it(VERTEX2F(7504,800));
    ft801_api_cmd_append_it(END());
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(COLOR_RGB(255,255,255));
    
    ft801_api_cmd_text_it(26,43,26, 1536, "100 mW");
    
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(LINE_WIDTH(32));
    ft801_api_cmd_append_it(COLOR_RGB(0,170,0));
    ft801_api_cmd_append_it(SCISSOR_XY(38,5));
    ft801_api_cmd_append_it(SCISSOR_SIZE(429,231));
    ft801_api_cmd_append_it(BEGIN(FT_LINE_STRIP));
    ft801_api_cmd_append_it(VERTEX2F(688,2656));
    ft801_api_cmd_append_it(VERTEX2F(880,2080));
    ft801_api_cmd_append_it(VERTEX2F(1312,640));
    ft801_api_cmd_append_it(VERTEX2F(1520,1072));
    ft801_api_cmd_append_it(VERTEX2F(1968,1456));
    ft801_api_cmd_append_it(VERTEX2F(2368,1808));
    ft801_api_cmd_append_it(VERTEX2F(2880,2224));
    ft801_api_cmd_append_it(VERTEX2F(3264,2288));
    ft801_api_cmd_append_it(VERTEX2F(3648,1840));
    ft801_api_cmd_append_it(VERTEX2F(3984,1424));
    ft801_api_cmd_append_it(VERTEX2F(4272,1696));
    ft801_api_cmd_append_it(VERTEX2F(4880,1872));
    ft801_api_cmd_append_it(VERTEX2F(5200,2224));
    ft801_api_cmd_append_it(VERTEX2F(5824,2224));
    ft801_api_cmd_append_it(VERTEX2F(6336,1696));
    ft801_api_cmd_append_it(VERTEX2F(6672,1488));
    ft801_api_cmd_append_it(VERTEX2F(7456,1728));
    ft801_api_cmd_append_it(END());
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    
    
    ft801_api_cmd_append_it(DISPLAY());
    ft801_api_cmd_append_it(CMD_SWAP);
    ft801_api_cmd_flush_it() ;
    
    return true ;
}

bool demoGraphTask_doing (void * const data)
{
    return true ;
}


bool demoGraphTask_gpuit(const uint8_t itflags, void * const data)
{
    return true ;
}



// DEMO:Setting task functions:

bool demoSettTask_painting (void * const data)
{
    ft801_api_cmd_prepare_it(FT_RAM_CMD) ;
    ft801_api_cmd_append_it(CMD_DLSTART) ;
    
    ft801_api_cmd_append_it(CLEAR_COLOR_RGB(0,0,0));
    ft801_api_cmd_append_it(CLEAR(1,1,1));
    
    
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(POINT_SIZE(160));
    ft801_api_cmd_append_it(COLOR_RGB(255,255,255));
    ft801_api_cmd_append_it(BEGIN(FT_POINTS));
    ft801_api_cmd_append_it(VERTEX2F(6960,848));
    ft801_api_cmd_append_it(END());
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(POINT_SIZE(648));
    ft801_api_cmd_append_it(COLOR_RGB(0,255,0));
    ft801_api_cmd_append_it(BEGIN(FT_POINTS));
    ft801_api_cmd_append_it(VERTEX2F(6976,3600));
    ft801_api_cmd_append_it(END());
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(POINT_SIZE(496));
    ft801_api_cmd_append_it(COLOR_RGB(0,0,0));
    ft801_api_cmd_append_it(BEGIN(FT_POINTS));
    ft801_api_cmd_append_it(VERTEX2F(6976,3600));
    ft801_api_cmd_append_it(END());
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(COLOR_RGB(0,85,255));
    
    ft801_api_cmd_fgcolor_it(0xffffff);
    ft801_api_cmd_bgcolor_it(0x383838);
    ft801_api_cmd_slider_it(16,46, 256,6, 0,85, 125);
    
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(COLOR_RGB(0,85,255));
    
    ft801_api_cmd_text_it(296,49,26, 1536, "85 C");
    
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(COLOR_RGB(255,255,255));
    
    ft801_api_cmd_text_it(164,14,28, 1536, "Krytyczna wartosc temperatury");
    
        
    ft801_api_cmd_text_it(120,81,28, 1536, "Dodatkowe chlodzenie");
       
    ft801_api_cmd_fgcolor_it(0xffffff);
    ft801_api_cmd_bgcolor_it(0x007efd);
    ft801_api_cmd_toggle_it(93,109,35,26,0,65535,"Wl" "\xff" "Wyl");
    
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(COLOR_RGB(0,85,255));
    
    ft801_api_cmd_bgcolor_it(0x002040);
    ft801_api_cmd_gauge_it(108,217,62,4352, 5,5,80,100);
    
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(COLOR_RGB(255,255,255));
    
    ft801_api_cmd_text_it(117,148,28, 1536, "Podswietlenie ekranu");
       
    ft801_api_cmd_text_it(107,230,26, 1536, "80%");
    
    ft801_api_cmd_text_it(435,226,28, 1536, "Wroc");
    
    RESTORE_CONTEXT();
    
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(LINE_WIDTH(160));
    ft801_api_cmd_append_it(COLOR_RGB(0,85,255));
    ft801_api_cmd_append_it(BEGIN(FT_LINES));
    ft801_api_cmd_append_it(VERTEX2F(6272,-160));
    ft801_api_cmd_append_it(VERTEX2F(7776,1264));
    ft801_api_cmd_append_it(END());
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(LINE_WIDTH(160));
    ft801_api_cmd_append_it(COLOR_RGB(0,85,255));
    ft801_api_cmd_append_it(BEGIN(FT_LINES));
    ft801_api_cmd_append_it(VERTEX2F(6768,32));
    ft801_api_cmd_append_it(VERTEX2F(7648,832));
    ft801_api_cmd_append_it(END());
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(COLOR_RGB(0,85,255));
    ft801_api_cmd_append_it(LINE_WIDTH(16));
    ft801_api_cmd_append_it(BEGIN(FT_RECTS));
    ft801_api_cmd_append_it(VERTEX2F(6976,16));
    ft801_api_cmd_append_it(VERTEX2F(7680,688));
    ft801_api_cmd_append_it(END());
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    ft801_api_cmd_append_it(SAVE_CONTEXT());
    ft801_api_cmd_append_it(LINE_WIDTH(32));
    ft801_api_cmd_append_it(COLOR_RGB(255,255,255));
    ft801_api_cmd_append_it(BEGIN(FT_LINES));
    ft801_api_cmd_append_it(VERTEX2F(6960,784));
    ft801_api_cmd_append_it(VERTEX2F(6960,2928));
    ft801_api_cmd_append_it(END());
    ft801_api_cmd_append_it(RESTORE_CONTEXT());
    
    
    
    ft801_api_cmd_append_it(DISPLAY());
    ft801_api_cmd_append_it(CMD_SWAP);
    ft801_api_cmd_flush_it() ;
    
    return true ;
}



bool demoSettTask_doing(void * const data)
{
    return true ;
}


bool demoSettTask_gpuit(const uint8_t itflags, void * const data)
{
    return true ;
}





// settings task



