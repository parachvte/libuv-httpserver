## libuv-httpserver

* Author: Ryan
* Last modified: 2014.12.23


A simple web server based on libuv and http-parser. (only echo `Hello, World!\r\n`)


### Dependencies

It takes lastest stable versions:

* [Libuv](https://github.com/libuv/libuv), Release v1.0.2 (released on 2014.12.10)
* [http-parser](https://github.com/joyent/http-parser), Release v2.3 (released on 2014.5.13)


### Benchmark

ab on My Macbook Air (8G, 1.3 GHz): `ab -n 10000 -c 100 http://127.0.0.1:8888/`

```
Document Path:          /
Document Length:        16 bytes

Concurrency Level:      100
Time taken for tests:   0.855 seconds
Complete requests:      10000
Failed requests:        0
Total transferred:      810000 bytes
HTML transferred:       160000 bytes
Requests per second:    11696.29 [#/sec] (mean)
Time per request:       8.550 [ms] (mean)
Time per request:       0.085 [ms] (mean, across all concurrent requests)
Transfer rate:          925.19 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        2    4   0.8      4      15
Processing:     2    4   1.2      4      15
Waiting:        1    4   1.2      4      15
Total:          6    8   1.6      8      20

Percentage of the requests served within a certain time (ms)
  50%      8
  66%      9
  75%      9
  80%     10
  90%     10
  95%     11
  98%     12
  99%     19
 100%     20 (longest request)
```
