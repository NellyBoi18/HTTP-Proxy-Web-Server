# Introduction

In this project, I created a proxy web server using HTTP. The Hypertext Transport Protocol (HTTP) is the most commonly used application protocol on the Internet today. Like many network protocols, HTTP uses a client-server model. An HTTP client opens a network connection to an HTTP server and sends an HTTP request message. Then, the server replies with an HTTP response message, which usually contains some resource (file, text, binary data) that was requested by the client.

I implemented an HTTP proxy server that handles HTTP GET requests. I provided functionality through the use of HTTP response headers, added support for HTTP error codes, and passed the request to an HTTP file server. The proxy server will wait for the response from the file server, and then forward the response to the client.

I also implemented a Priority Queue for the jobs sent to the proxy server. This proxy server will also implement functionality that will allow the client to query it for job status and order in the priority queue.

# Background

## Structure of an HTTP Request
The format of a HTTP request message is:
- an HTTP request line (containing a method, a query string, and the HTTP protocol version) 
- zero or more HTTP header lines
- a blank line (i.e. a CRLF by itself)

The line ending used in HTTP requests is CRLF, which is represented as \r\n in C.
Below is an example HTTP request message sent by the Google Chrome browser to a HTTP web server
running on localhost (127.0.0.1) on port 8000 (the CRLF’s are written out using their escape sequences):

```
GET /hello.html HTTP/1.0\r\n
Host: 127.0.0.1:8000\r\n
Connection: keep-alive\r\n
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n
User-Agent: Chrome/45.0.2454.93\r\n
Accept-Encoding: gzip,deflate,sdch\r\n
Accept-Language: en-US,en;q=0.8\r\n
\r\n
```

Header lines provide information about the request. Here are some HTTP request header types:
- Host: contains the hostname part of the URL of the HTTP request (e.g. inst.eecs.berkeley.edu
or 127.0.0.1:8000)
- User-Agent: identifies the HTTP client program, takes the form Program-name/x.xx, where x.xx is the version of the program. In the above example, the Google Chrome browser sets User- Agent as Chrome/45.0.2454.93.

## Structure of an HTTP Response

The format of a HTTP response message is:
- an HTTP response status line (containing the HTTP protocol version, the status code, and a description of the status code)
- zero or more HTTP header lines
- a blank line (i.e. a CRLF by itself)
- the content requested by the HTTP request

The line ending used in HTTP requests is CRLF, which is represented as \r\n in C.
Here is a example HTTP response with a status code of 200 and an HTML file attached to the response
(the CRLF’s are written out using their escape sequences):

```
HTTP/1.0 200 OK\r\n
Content-Type: text/html\r\n
Content-Length: 128\r\n
\r\n
<html>\n
<body>\n
<h1>Hello World</h1>\n
<p>\n
Let’s see if this works\n
</p>\n
</body>\n
</html>\n
```

Typical status lines might be HTTP/1.0 200 OK (as in our example above), HTTP/1.0 404 Not Found, etc.
The status code is a three-digit integer, and the first digit identifies the general category of response:
- 1xx indicates an informational message only 
- 2xx indicates success
- 3xx redirects the client to another URL
- 4xx indicates an error in the client
- 5xx indicates an error in the server

Header lines provide information about the response. Here are some HTTP response header types:
- Content-Type: the MIME type of the data attached to the response, such as text/html or text/plain
- Content-Length: the number of bytes in the body of the response

# Project details

## Create a basic proxy web server

I created a proxy web server. From a network standpoint, the server implements the following:
1. Create a listening socket and bind it to a port.
2. Wait for clients to connect to the port.
3. Accept the client and obtain a new connection socket
4. Read in and parse the HTTP request
5. Add the request to your priority queue in priority order (More details below).
6. Worker threads will pick up a request from the queue in priority order and send it to the target server.
7. Once the response is received from the target server, send it back to the client (or send an error message).

## Running the Proxy Server

Here is the usage string for proxyserver.

```
$ ./proxyserver -l <num listeners> <ports list> 
                -w <num workers> 
                -p <port of target file server> 
                -i <IP address of target file server> 
                -q <max number of requests in the priority queue> 
```

For example,

```
$ ./proxyserver -l 1 33489 -w 1 -i 127.0.0.1 -p 57455 -q 10
```

The available options are:
- -l : The number of listening threads, and a list of the ports for each of these threads. These threads will listen for clients trying to connect to the proxy server, and once they receive a request, will add the request to the priority queue. 
- -w : Indicates the number of threads in your thread pool that are able to concurrently process client requests. These threads will take requests from the queue and process them, and then return responses to the client.
- -p : Port number for the target file server.
- -i : IP address of the target file server.
- -q : maximum number of requests that can be added to the priority queue. If more requests arrive, it will return an error code to the user, and that request will be dropped. 


## Accessing a Webserver

You can send HTTP requests with the curl program. An example of how to use curl is:
```
    $ curl -v http://192.168.162.162:8000/
    $ curl -v http://192.168.162.162:8000/index.html
    $ curl -v http://192.168.162.162:8000/path/to/file
```

You can run this program by starting a simple fileserver using Python3, and running the following commands:

```
    Start the file server inside the public_html folder
    $ python3 -m http.server 57455

    Start your proxyserver
    $ ./proxyserver -l 1 33489 -w 1 -i 127.0.0.1 -p 57455 -q 10

    Send a request to the proxy server
    $ curl 'http://localhost:33489/1/dummy1.html'

    Send a GetJob request to your proxyserver
    curl 'http://localhost:33489/GetJob'
```

I have also provided some scripts to run the servers
- `start.py`:  
    - Use to start the test client, the proxy server, and the target server, and pass it all the relevant information needed to start the proxy server.
    - You can run `./start.py -h` to see the usage of this script. 
    - Most of the arguments are the same as the ones described to start the proxy server
- `clean.sh`
    - This can be used to clean up any ports or other network resources.


## Common Error Messages

### Failed to bind on socket: Address already in use

This means you have a proxyserver running in the background. This can happen if your code leaks processes that hold on to their sockets, or if you disconnected from the lab machine and never shut down your server. You can fix this by running “pkill -9 proxyserver”. If that doesn’t work, you can specify a different port by running “proxyserver --files files/ --port 8001”.

### Failed to bind on socket: Permission denied
If you use a port number that is less than 1024, you may receive this error. Only the root user can use the “well-known” ports (numbers 1 to 1023), so you should choose a higher port number (1024 to 65535).

# Priority Queue for Jobs 

## What is a Priority Queue?

A priority queue is a type of queue that arranges elements based on their priority values. Elements with higher priority values are always retrieved before elements with lower priority values.

In a priority queue, each element has a priority value associated with it. When you add an element to the queue, it is inserted in a position based on its priority value. For example, if you add an element with a high priority value to a priority queue, it will be inserted near the front of the queue, while an element with a low priority value will be inserted near the back. (Max priority queue - descending order of pri)

There are several ways to implement a priority queue, including using an array, linked list, heap, or binary search tree.

The main properties of a priority queue are:

- Every item has a priority associated with it.
- An element with high priority is dequeued before an element with low priority.
- If two elements have the same priority, they can be serviced in any order.

## Proxy Server Priority Queue

In this project, I implemented a priority queue to track the jobs sent to the proxy server from the clients. As jobs come in, they will be added to the queue according to the priority which is based on which directory is being accessed. Each directory is assigned a priority, and the program parses the path provided to figure out the priority of the request.

You can use any implementation of a priority queue, but we recommend using a heap (with an array as the underlying structure for the heap) as it will give the best performance for this particular application. 

I have implemented the following interface:

- `create_queue()`: Create a new priority queue.
- `add_work()`: When a new request comes in, it is inserted in priority order.
- `get_work()`: Get the job with highest priority.
    - Two versions of the remove functionality: One that will be called by worker threads when they want to get a new job to work on, and another that is called by listener threads when they receive a "GetJob" request.
    - `get_work()`: The worker threads will call a Blocking version of remove, where if there are no elements in the queue, they will block until an item is added. This is implemented using condition variables.
    - `get_work_nonblocking()`: The listener threads should call a Non-Blocking function to get the highest priority job. If there are no elements on the queue, they will simply return and send an error message to the client.

The proxy server also handles the special request type "GetJob" which will be used by the tests to ensure that the order of the queue is consistent. These tests will do the following:
- If there are n worker threads, the clients will send n requests with certain amount of delay.
- This will cause all the worker threads to sleep for the duration of the delays specified.
- Any new requests that are sent by the client will be inserted into the priority queue, but will not be immediately handled by a worker thread.
- Then, the client will issue "GetJob" requests to remove the highest priority job from the queue in priority order.

# Threading and Synchronization

This program uses the pthreads C library to create multiple listener and worker threads.
