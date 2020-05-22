#include <image_processing.h>

void IP_FromCamera(     int8_t* destination_buffer, 
                        Camera &camera,
                        unsigned int destination_height, 
                        unsigned int destination_width,
                        bool white_balance)
{
    unsigned int start_x = (camera.get_width() - destination_width)/2;
    unsigned int start_y = (camera.get_height() - destination_height)/2; 
    
    int mean = 0;

    if (white_balance)
    {
        unsigned int pixels = 0;  

        for (unsigned int y = 0; y < destination_height; y++)
            for (unsigned int x = 0; x < destination_width; x++)
            {
                unsigned int    idx_input = (y + start_y)*camera.get_width() + x + start_x;
                int             pixel     = ((((uint16_t*)camera.get_buffer())[idx_input])&0xFF00)>>8;
                
                mean+=          pixel;
                pixels++;
            }
            
        mean = mean / pixels; 
    }

    unsigned int ptr = 0;
     
    for (unsigned int y = 0; y < destination_height; y++)
        for (unsigned int x = 0; x < destination_width; x++)
        {
            unsigned int    idx_input = (y + start_y)*camera.get_width() + x + start_x;
            int             pixel     = ((((uint16_t*)camera.get_buffer())[idx_input])&0xFF00)>>8;

            int v                     = pixel - mean;
            
            if (v > 127) 
                v = 127;

            if (v < -127)
                v = -127; 

            destination_buffer[ptr] = v;
            ptr++;
        } 
}



void IP_ToDisplay(  LCD &lcd,
                    int8_t* source_buffer, 
                    unsigned int source_height, 
                    unsigned int source_width,
                    unsigned int y_ofs,
                    unsigned int x_ofs)
{
    unsigned int ptr = 0;
    for (unsigned int y = 0; y < source_height; y++)
        for (unsigned int x = 0; x < source_width; x++)
        {
            unsigned int intensity = source_buffer[ptr] + 127;
            ptr++;

            lcd.DrawPixel(x + x_ofs, y + y_ofs, intensity, intensity, intensity);
        }
}

void IP_ResultToDisplay(    LCD &lcd,
                            int8_t* source_buffer, 
                            unsigned int source_height, 
                            unsigned int source_width,
                            unsigned int upscale,
                            unsigned int y_ofs,
                            unsigned int x_ofs)
{
    unsigned int threshold = 150;
    unsigned int ptr = 0;
    for (unsigned int y = 0; y < source_height; y++)
        for (unsigned int x = 0; x < source_width; x++)
        {
            unsigned int q00 = source_buffer[((y + 0)%source_height)*source_width + (x + 0)%source_width] + 127;
            unsigned int q01 = source_buffer[((y + 0)%source_height)*source_width + (x + 1)%source_width] + 127;
            unsigned int q10 = source_buffer[((y + 1)%source_height)*source_width + (x + 0)%source_width] + 127;
            unsigned int q11 = source_buffer[((y + 1)%source_height)*source_width + (x + 1)%source_width] + 127;

            if (q00 > threshold)
                q00 = 255;
            else
                q00 = 0;

            if (q01 > threshold)
                q01 = 255;
            else
                q01 = 0;

            if (q10 > threshold)
                q10 = 255;
            else
                q10 = 0; 

            if (q11 > threshold)
                q11 = 255;
            else
                q11 = 0;


            ptr++;
            
            for (unsigned int ky = 0; ky < upscale; ky++)
                for (unsigned int kx = 0; kx < upscale; kx++)
                {
                    unsigned int intensity = 0;
                    
                    intensity+= q00*(upscale - ky)*(upscale - kx);
                    intensity+= q01*(upscale - ky)*kx;
                    intensity+= q10*ky*(upscale - kx);
                    intensity+= q11*ky*kx; 
                    
                    intensity/= upscale*upscale;
                    
                    lcd.DrawPixel(x*upscale + kx + x_ofs, y*upscale + ky + y_ofs, intensity, intensity, intensity);
                }
        }
}