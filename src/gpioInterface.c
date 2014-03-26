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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <stdint.h>


#define GPIO_NO_EDGE (0)
#define GPIO_RISING_EDGE (1)
#define GPIO_FALLING_EDGE (2)
#define GPIO_BOTH_EDGES (3)

 /****************************************************************
 * Constants
 ****************************************************************/
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define MAX_BUF 64

/*******************************************************************************
 * This method will export the given GPIO pin.
 * @param uint32_t gpio - This is the pin that is to be exported.
 * @return The return will be 0 if successful or a negative number if an error occurs.
 ******************************************************************************/
int32_t gpio_export(uint32_t gpio)
{
	int32_t fd, len;
	char buf[MAX_BUF];
 
	fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}
 
	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
 
	return 0;
}

/*******************************************************************************
 * This method will unexport the given GPIO pin.
 * @param uint32_t gpio - This is the pin that is to be exported.
 * @return The return will be 0 if successful or a negative number if an error occurs.
 ******************************************************************************/
int32_t gpio_unexport(uint32_t gpio)
{
	int32_t fd, len;
	char buf[MAX_BUF];
 
	fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}
 
	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
	return 0;
}

/*******************************************************************************
 * This method will set the direction of the given GPIO pin.
 * @param uint32_t gpio - This is the pin that is to be set.
 * @param uint32_t out_flag This is the direction.  A 0 value indicates input.  
 *                          A nonzero value is output.
 * @return The return will be 0 if successful or a negative number if an error occurs.
 ******************************************************************************/
int32_t gpio_set_dir(uint32_t gpio, uint32_t out_flag)
{
	int32_t fd, len;
	char buf[MAX_BUF];
 
	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/direction");
		return fd;
	}
 
	if (out_flag)
	{	
		write(fd, "out", 4);
	}
	else
	{
		write(fd, "in", 3);
	}
 
	close(fd);
	return 0;
}

/*******************************************************************************
 * This method will set the value of the given GPIO pin.
 * @param uint32_t gpio - This is the pin that is to be set.
 * @param uint32_t value This is the value.  A 0 value is lo / off.  A non zero value is high / on.
 * @return The return will be 0 if successful or a negative number if an error occurs.
 ******************************************************************************/
int32_t gpio_set_value(uint32_t gpio, uint32_t value)
{
	int32_t fd, len;
	char buf[MAX_BUF];
 
	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-value");
		return fd;
	}
 
	if (value)
	{
		write(fd, "1", 2);
	}
	else
	{	
		write(fd, "0", 2);
	}
 
	close(fd);
	return 0;
}

/*******************************************************************************
 * This method will read the value set on a given pin.
 * @param uint32_t gpio - This is the pin that is to be read.
 * @param uint32_t * value This is a pointer to where the value is to be placed when it is read.
 * @return The return will be 0 if successful or a negative number if an error occurs.
 ******************************************************************************/
int32_t gpio_get_value(uint32_t gpio, uint32_t *value)
{
	int32_t fd, len;
	char buf[MAX_BUF];
	char ch;

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
 
	fd = open(buf, O_RDONLY);
	if (fd < 0) {
		perror("gpio/get-value");
		return fd;
	}
 
	read(fd, &ch, 1);

	if (ch != '0') {
		*value = 1;
	} else {
		*value = 0;
	}
 
	close(fd);
	return 0;
}



/*******************************************************************************
 * This method will set the edge upon which an interrupt is fired.
 * @param uint32_t gpio - This is the pin that is to be configured.
 * @param uint32_t edgeType This is the edge type.  It can be either 
 * GPIO_NO_EDGE, GPIO_RISING_EDGE, GPIO_FALLING_EDGE, or GPIO_BOTH_EDGES.
 * @return The return will be 0 if successful or a negative number if an error occurs.
 ******************************************************************************/
 int32_t gpio_set_edge(uint32_t gpio, uint32_t edgeType)
{
	int32_t fd, len;
	char buf[MAX_BUF];
	char* edgetypes[] = {"none", "rising", "falling", "both"};

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/edge", gpio);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-edge");
		return fd;
	}
 
	write(fd, edgetypes[edgeType], strlen(edgetypes[edgeType]) + 1); 
	close(fd);
	return 0;
}

/*******************************************************************************
 * This method open the given file.
 * @param uint32_t gpio - This is the pin that is to be opened.
 * @return The return will be a file id for the opened file.
 ******************************************************************************/
int32_t gpio_fd_open(uint32_t gpio)
{
	int32_t fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
 
	fd = open(buf, O_RDONLY | O_NONBLOCK );
	if (fd < 0) {
		perror("gpio/fd_open");
	}
	return fd;
}

/*******************************************************************************
 * This method close the given file descriptor.
 * @param uint32_t fd This is the file descriptor that is to be closed.
 * @return The function returns 0 if successful, -1 to indicate an error, with 
 * errno set appropriately.
 ******************************************************************************/
int32_t gpio_fd_close(int32_t fd)
{
	return close(fd);
}

