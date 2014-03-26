/*********************************************************************
 * This interface will aid the user in working with GPIO pins.  
 * It will allow the user to setup a pin as either input or output and 
 * return a file pointer to that pin. The code was initially developed by Dingo_aus, 7 January 2009
 * email: dingo_aus [at] int32_ternode <dot> on /dot/ net
 * From http://www.avrfreaks.net/wiki/index.php/Documentation:LinuxGPIO#gpio_framework
 *
 * Created in AVR32 Studio (version 2.0.2) running on Ubuntu 8.04
 * Modified by Mark A. Yoder, 21-July-2011
 * Refactored and further modified by Walter Schilling, Spring 2013-2014.
 */

#ifndef GPIOINTERFACE_H
#define GPIOINTERFACE_H
 
#include <stdint.h>
#define GPIO_NO_EDGE (0)
#define GPIO_RISING_EDGE (1)
#define GPIO_FALLING_EDGE (2)
#define GPIO_BOTH_EDGES (3)

/*******************************************************************************
 * This method will export the given GPIO pin.
 * @param uint32_t gpio - This is the pin that is to be exported.
 * @return The return will be 0 if successful or a negative number if an error occurs.
 ******************************************************************************/
int32_t gpio_export(uint32_t gpio);

/*******************************************************************************
 * This method will unexport the given GPIO pin.
 * @param uint32_t gpio - This is the pin that is to be exported.
 * @return The return will be 0 if successful or a negative number if an error occurs.
 ******************************************************************************/
int32_t gpio_unexport(uint32_t gpio);

/*******************************************************************************
 * This method will set the direction of the given GPIO pin.
 * @param uint32_t gpio - This is the pin that is to be set.
 * @param uint32_t out_flag This is the direction.  A 0 value indicates input.  
 *                          A nonzero value is output.
 * @return The return will be 0 if successful or a negative number if an error occurs.
 ******************************************************************************/
int32_t gpio_set_dir(uint32_t gpio, uint32_t out_flag);

/*******************************************************************************
 * This method will set the value of the given GPIO pin.
 * @param uint32_t gpio - This is the pin that is to be set.
 * @param uint32_t value This is the value.  A 0 value is lo / off.  A non zero value is high / on.
 * @return The return will be 0 if successful or a negative number if an error occurs.
 ******************************************************************************/
int32_t gpio_set_value(uint32_t gpio, uint32_t value);

/*******************************************************************************
 * This method will read the value set on a given pin.
 * @param uint32_t gpio - This is the pin that is to be read.
 * @param uint32_t * value This is a pointer to where the value is to be placed when it is read.
 * @return The return will be 0 if successful or a negative number if an error occurs.
 ******************************************************************************/
int32_t gpio_get_value(uint32_t gpio, uint32_t *value);

/*******************************************************************************
 * This method will set the edge upon which an interrupt is fired.
 * @param uint32_t gpio - This is the pin that is to be configured.
 * @param uint32_t edgeType This is the edge type.  It can be either 
 * GPIO_NO_EDGE, GPIO_RISING_EDGE, GPIO_FALLING_EDGE, or GPIO_BOTH_EDGES.
 * @return The return will be 0 if successful or a negative number if an error occurs.
 ******************************************************************************/
 int32_t gpio_set_edge(uint32_t gpio, uint32_t edgeType);

/*******************************************************************************
 * This method open the given file.
 * @param uint32_t gpio - This is the pin that is to be opened.
 * @return The return will be a file id for the opened file.
 ******************************************************************************/
int32_t gpio_fd_open(uint32_t gpio);

/*******************************************************************************
 * This method close the given file descriptor.
 * @param uint32_t fd This is the file descriptor that is to be closed.
 * @return The function returns 0 if successful, -1 to indicate an error, with 
 * errno set appropriately.
 ******************************************************************************/
int32_t gpio_fd_close(int32_t fd);

#endif
