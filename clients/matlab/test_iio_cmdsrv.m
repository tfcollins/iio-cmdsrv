%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Copyright 2012 Analog Devices Inc.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

clear all;
close all;

obj = iio_cmdsrv;

iio_cmdsrv_connect(obj, '192.168.11.153', 4321);

[ret, rbuf] = iio_cmd_read(obj, 200, 'read cf-ad9643-core-lpc name\n');
sprintf('\nread cf-ad9643-core-lpc name --> return: %d, data: %s\n', ret, rbuf)

[ret, rbuf] = iio_cmd_read(obj, 200, 'read cf-ad9643-core-lpc in_voltage0_test_mode\n');
sprintf('\nread cf-ad9643-core-lpc in_voltage0_test_mode --> return: %d, data: %s\n', ret, rbuf)

[ret, rbuf] = iio_cmd_read(obj, 1000, 'read cf-ad9643-core-lpc in_voltage_scale_available\n');
sprintf('\nread cf-ad9643-core-lpc in_voltage_scale_available --> return: %d, data: %s\n', ret, rbuf)

[ret, rbuf] = iio_cmd_read(obj, 200, 'read cf-ad9643-core-lpc in_voltage_scale\n');
sprintf('\nread cf-ad9643-core-lpc in_voltage_scale --> return: %d, data: %s\n', ret, rbuf)

[ret, rbuf] = iio_cmd_read(obj, 200, 'read cf-ad9643-core-lpc name\n');
sprintf('\nread cf-ad9643-core-lpc name --> return: %d, data: %s\n', ret, rbuf)

ret = iio_cmd_send(obj, 'write cf-ad9643-core-lpc in_voltage_scale %f\n', 0.025024);
sprintf('\nwrite cf-ad9643-core-lpc in_voltage_scale %f --> return: %d\n', 0.025024, ret)

[ret, rbuf] = iio_cmd_read(obj, 200, 'read cf-ad9643-core-lpc in_voltage_scale\n');
sprintf('\nread cf-ad9643-core-lpc in_voltage_scale --> return: %d, data: %s\n', ret, rbuf)

ret = iio_cmd_send(obj, 'write cf-ad9643-core-lpc in_voltage_scale %f\n', 0.030060);
sprintf('\nwrite cf-ad9643-core-lpc in_voltage_scale %f --> return: %d\n', 0.030060, ret)

[ret, rbuf] = iio_cmd_read(obj, 200, 'read cf-ad9643-core-lpc in_voltage_scale\n');
sprintf('\nread cf-ad9643-core-lpc in_voltage_scale --> return: %d, data: %s\n', ret, rbuf)

[ret, rbuf] = iio_cmd_sample(obj, 'cf-ad9643-core-lpc', 40, 4);
sprintf('\niio_cmd_sample cf-ad9643-core-lpc 40, 4 --> return: %d, data: \n', ret) 
uint8(rbuf)

[ret, val] = iio_cmd_regread(obj, 'cf-ad9643-core-lpc', 1);
sprintf('\niio_cmd_regread cf-ad9643-core-lpc 1 --> return: %d, data: %d\n', ret, val)

[ret, val] = iio_cmd_regread(obj, 'cf-ad9643-core-lpc', 2);
sprintf('\niio_cmd_regread cf-ad9643-core-lpc 2 --> return: %d, data: %d\n', ret, val)

[ret, val] = iio_cmd_regread(obj, 'cf-ad9643-core-lpc', 1);
sprintf('\niio_cmd_regread cf-ad9643-core-lpc 1 --> return: %d, data: %d\n', ret, val)

ret = iio_cmd_regwrite(obj, 'cf-ad9643-core-lpc', hex2dec('20'), hex2dec('AB'));
sprintf('\niio_cmd_regwrite cf-ad9643-core-lpc 0x20, 0xAB --> return: %d\n', ret)

[ret, val] = iio_cmd_regread(obj, 'cf-ad9643-core-lpc', hex2dec('20'));
sprintf('\niio_cmd_regread cf-ad9643-core-lpc 0x20 --> return: %d, data: %d\n', ret, val)

ret = iio_cmd_regwrite(obj, 'cf-ad9643-core-lpc', hex2dec('20'), 0);
sprintf('\niio_cmd_regwrite cf-ad9643-core-lpc 0x20 0 --> return: %d\n', ret)

[ret, val] = iio_cmd_regread(obj, 'cf-ad9643-core-lpc', hex2dec('20'));
sprintf('\niio_cmd_regread cf-ad9643-core-lpc --> return: %d, data: %d\n', ret, val)

% buf = zeros(1, 400000);
% ret = iio_cmd_bufwrite(obj, 'cf-ad9643-core-lpc', buf, 400000);
% sprintf('\niio_cmd_bufwrite cf-ad9643-core-lpc 400000 --> return: %d\n', ret)

ret = iio_cmdsrv_disconnect(obj);
sprintf('\niio_cmdsrv_disconnect --> return: %d\n', ret)
