#include <stdio.h>
#include <stdlib.h>

#include "lib_iio_cmdsrv.h"

#if 1
int main (int argc , char* argv[])
{
	struct iio_cmdsrv srv;
	int ret; unsigned i;
	short buf2[400000];
	char *buf = (char*) buf2;

	ret = iio_cmdsrv_connect("10.44.2.188", "1234", &srv);
	if (ret)
		perror("connection failed");

	ret = iio_cmd_read(&srv, buf, 200, "read cf-ad9643-core-lpc name\n", NULL);

	printf("main:retval %s (ret %d)\n", buf, ret);

	ret = iio_cmd_read(&srv, buf, 200, "read cf-ad9643-core-lpc in_voltage0_test_mode\n", NULL);
	printf("main:retval %s (ret %d)\n", buf, ret);

	ret = iio_cmd_read(&srv, buf, 1000, "read cf-ad9643-core-lpc in_voltage_scale_available\n", NULL);
	printf("main:retval %s (ret %d)\n", buf, ret);

	ret = iio_cmd_read(&srv, buf, 200, "read cf-ad9643-core-lpc in_voltage_scale\n", NULL);
	printf("main:retval %s (ret %d)\n", buf, ret);

	ret = iio_cmd_read(&srv, buf, 200, "read cf-ad9643-core-lpc name\n", NULL);
	printf("main:retval %s (ret %d)\n", buf, ret);

	ret = iio_cmd_send(&srv, "write cf-ad9643-core-lpc in_voltage_scale %f\n", 0.025024);
	printf("main:retval %s (ret %d)\n", buf, ret);


	ret = iio_cmd_read(&srv, buf, 200, "read cf-ad9643-core-lpc in_voltage_scale\n", NULL);
	printf("main:retval %s (ret %d)\n", buf, ret);

	ret = iio_cmd_send(&srv, "write cf-ad9643-core-lpc in_voltage_scale %f\n", 0.030060);
	printf("main:retval %s (ret %d)\n", buf, ret);

	ret = iio_cmd_read(&srv, buf, 200, "read cf-ad9643-core-lpc in_voltage_scale\n", NULL);
	printf("main:retval %s (ret %d)\n", buf, ret);

	ret = iio_cmd_sample(&srv, "cf-ad9643-core-lpc",
		   buf, 40, 4);
	printf("main:retval (ret %d)\n", ret);
	for (i = 0; i < 40; i++)
		printf("[%d] = %d\n", i, buf2[i]);


	ret = iio_cmd_sample(&srv, "cf-ad9643-core-lpc",
		   buf, 40, 4);
	printf("main:retval (ret %d)\n", ret);
	for (i = 0; i < 40; i++)
		printf("[%d] = %d\n", i, buf2[i]);

	ret = iio_cmd_regread(&srv, "cf-ad9643-core-lpc", 1, &i);
	printf("%d [%x]\n", ret, i);

	ret = iio_cmd_regread(&srv, "cf-ad9643-core-lpc", 2, &i);
	printf("%d [%x]\n", ret, i);

	ret = iio_cmd_regread(&srv, "cf-ad9643-core-lpc", 1, &i);
	printf("%d [%x]\n", ret, i);

	ret = iio_cmd_regwrite(&srv, "cf-ad9643-core-lpc", 0x20, 0xAB);
	printf("%d [%x]\n", ret, i);

	ret = iio_cmd_regread(&srv, "cf-ad9643-core-lpc", 0x20, &i);
	printf("%d [%x]\n", ret, i);

	ret = iio_cmd_regwrite(&srv, "cf-ad9643-core-lpc", 0x20, 0);
	printf("%d [%x]\n", ret, i);

	ret = iio_cmd_regread(&srv, "cf-ad9643-core-lpc", 0x20, &i);
	printf("%d [%x]\n", ret, i);

	ret = iio_cmd_bufwrite(&srv, "cf-ad9643-core-lpc",
		   buf, 400000);
	printf("main:retval (ret %d)\n", ret);




	return 0;

}
#endif