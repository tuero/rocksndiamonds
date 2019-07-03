
#ifndef CUSTOM_H
#define CUSTOM_H



#ifdef __cplusplus
extern "C"
{
#endif
    // C declarations
    #include "../main.h"
    #include "../screens.h"
    #include "../files.h"
    void findPath(void);
    void printBoardState(void);
    void printBoardDistances(void);
    void setLevel(int levelset); 
    int getAction(controller_type controller);
    void calcDistances(void);
    void testEngineSpeed(void);
    void testBFSSpeed(void);
    void testMCTSSpeed(void);

#ifdef __cplusplus
}
#endif






#endif  //CUSTOM_H


