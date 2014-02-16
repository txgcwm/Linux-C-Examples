#include "wav_parse.h"

int main(int argc, char **argv){
    wav_t *wav = NULL;


    if(argc != 2){
        printf("Usage:\n");
        printf("\twav_parse wav_file\n");
        return -1;
    }


    wav = wav_open(argv[1]);
    if(NULL != wav){
        wav_dump(wav);
        wav_close(&wav);
    }

    return 0;
}
