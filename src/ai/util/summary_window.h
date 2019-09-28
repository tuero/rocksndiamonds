/**
 * @file: summary_window.h
 *
 * @brief: Second window to display summarized information of planner and objects in game.
 * 
 * @author: Jake Tuero
 * Date: August 2019
 * Contact: tuero@ualberta.ca
 */

#ifndef SUMMARY_WINDOW
#define SUMMARY_WINDOW 

#include <vector>

namespace summarywindow {

    /*
     * Initialize the summary window.
     */
    bool init();

    /*
     * Close the summary window and cleanup renderer.
     */
    void close();

    /*
     * Update the stored grid positions of abstract nodes/objects
     */
    void updateGridRepresentation(std::vector<std::vector<int>> &gridRepresentation);

    /*
     * Draw the summary information.
     * Simplified board is drawn, along with planned abstractions and path.
     */
    void draw();
}


#endif  // SUMMARY_WINDOW