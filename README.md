# sese-http

## test command

http/1.1 to h2

```shell
curl --http2 -k http://127.0.0.1:80/haha --noproxy "*" -v
```

without upgrade

```shell
curl --http2-prior-knowledge -k https://127.0.0.1:80/haha --noproxy "*" -v
```

## todos

- servlet
- controller
- filter
- static resources mounting
- ALPN
- http/1.1
- http/2 (exclude the priority algorithm)
- merge into sese-core

## [RFC7540](https://www.rfc-editor.org/rfc/rfc7540.txt) verify by [h2spec](https://github.com/summerwind/h2spec)

> [!NOTE]
> Except for 6.1/2, 6.2/4,
> the unfinished parts are suspected to be due to errors in the implementation of the h2spec standard.

- 6.1. DATA
    - [x] 1: Sends a DATA frame with 0x0 stream identifier
    - [ ] 2: Sends a DATA frame on the stream that is not in "open" or "half-closed (local)" state
    - [x] 3: Sends a DATA frame with invalid pad length

- 6.2. HEADERS
    - [x] 1: Sends a HEADERS frame without the END_HEADERS flag, and a PRIORITY frame
    - [x] 2: Sends a HEADERS frame to another stream while sending a HEADERS frame
    - [x] 3: Sends a HEADERS frame with 0x0 stream identifier
    - [ ] 4: Sends a HEADERS frame with invalid pad length

- 6.3. PRIORITY
    - [x] 1: Sends a PRIORITY frame with 0x0 stream identifier
    - [x] 2: Sends a PRIORITY frame with a length other than 5 octets

- 6.4. RST_STREAM
    - [x] 1: Sends a RST_STREAM frame with 0x0 stream identifier
    - [x] 2: Sends a RST_STREAM frame on a idle stream
    - [x] 3: Sends a RST_STREAM frame with a length other than 4 octets

- 6.5. SETTINGS
    - [x] 1: Sends a SETTINGS frame with ACK flag and payload
    - [x] 2: Sends a SETTINGS frame with a stream identifier other than 0x0
    - [x] 3: Sends a SETTINGS frame with a length other than a multiple of 6 octets
    - 6.5.2. Defined SETTINGS Parameters
        - [x] 1: SETTINGS_ENABLE_PUSH (0x2): Sends the value other than 0 or 1
        - [x] 2: SETTINGS_INITIAL_WINDOW_SIZE (0x4): Sends the value above the maximum flow control window size
        - [x] 3: SETTINGS_MAX_FRAME_SIZE (0x5): Sends the value below the initial value
        - [x] 4: SETTINGS_MAX_FRAME_SIZE (0x5): Sends the value above the maximum allowed frame size
        - [ ] 5: Sends a SETTINGS frame with unknown identifier
    - 6.5.3. Settings Synchronization
        - [ ] 1: Sends multiple values of SETTINGS_INITIAL_WINDOW_SIZE
        - [x] 2: Sends a SETTINGS frame without ACK flag

- 6.7. PING
    - [x] 1: Sends a PING frame
    - [x] 2: Sends a PING frame with ACK
    - [x] 3: Sends a PING frame with a stream identifier field value other than 0x0
    - [x] 4: Sends a PING frame with a length field value other than 8

- 6.8. GOAWAY
    - [x] 1: Sends a GOAWAY frame with a stream identifier other than 0x0

- 6.9. WINDOW_UPDATE
    - [x] 1: Sends a WINDOW_UPDATE frame with a flow control window increment of 0
    - [x] 2: Sends a WINDOW_UPDATE frame with a flow control window increment of 0 on a stream
    - [x] 3: Sends a WINDOW_UPDATE frame with a length other than 4 octets
    - 6.9.1. The Flow-Control Window
        - [ ] 1: Sends SETTINGS frame to set the initial window size to 1 and sends HEADERS frame
        - [x] 2: Sends multiple WINDOW_UPDATE frames increasing the flow control window to above 2^31-1
        - [x] 3: Sends multiple WINDOW_UPDATE frames increasing the flow control window to above 2^31-1 on a stream
    - 6.9.2. Initial Flow-Control Window Size
        - [ ] 1: Changes SETTINGS_INITIAL_WINDOW_SIZE after sending HEADERS frame
        - [ ] 2: Sends a SETTINGS frame for window size to be negative
        - [ ] 3: Sends a SETTINGS_INITIAL_WINDOW_SIZE settings with an exceeded maximum window size value

- 6.10. CONTINUATION
    - [x] 1: Sends multiple CONTINUATION frames preceded by a HEADERS frame
    - [x] 2: Sends a CONTINUATION frame followed by any frame other than CONTINUATION
    - [x] 3: Sends a CONTINUATION frame with 0x0 stream identifier
    - [x] 4: Sends a CONTINUATION frame preceded by a HEADERS frame with END_HEADERS flag
    - [x] 5: Sends a CONTINUATION frame preceded by a CONTINUATION frame with END_HEADERS flag
    - [x] 6: Sends a CONTINUATION frame preceded by a DATA frame

## HTTP2 Server verify by h2spec

- 1.Starting HTTP/2
    - [x] 1: Sends a client connection preface
- 2.Streams and Multiplexing
    - [x] 1: Sends a PRIORITY frame on idle stream
    - [ ] 2: Sends a WINDOW_UPDATE frame on half-closed (remote) stream
    - [ ] 3: Sends a PRIORITY frame on half-closed (remote) stream
    - [ ] 4: Sends a RST_STREAM frame on half-closed (remote) stream
    - [x] 5: Sends a PRIORITY frame on closed stream

- 3.Frame Definitions
    - 3.1. DATA
      - [x] 1: Sends a DATA frame
      - [x] 2: Sends multiple DATA frames
      - [x] 3: Sends a DATA frame with padding
    - 3.2. HEADERS
      - [x] 1: Sends a HEADERS frame
      - [x] 2: Sends a HEADERS frame with padding
      - [x] 3: Sends a HEADERS frame with priority
    - 3.3. PRIORITY
      - [x] 1: Sends a PRIORITY frame with priority 1
      - [x] 2: Sends a PRIORITY frame with priority 256
      - [x] 3: Sends a PRIORITY frame with stream dependency
      - [x] 4: Sends a PRIORITY frame with exclusive
      - [x] 5: Sends a PRIORITY frame for an idle stream, then send a HEADER frame for a lower stream ID
    - 3.4. RST_STREAM
      - [x] 1: Sends a RST_STREAM frame
    - 3.5. SETTINGS
      - [x] 1: Sends a SETTINGS frame
    - 3.7. PING
      - [x] 1: Sends a PING frame
    - 3.8. GOAWAY
      - [x] 1: Sends a GOAWAY frame
    - 3.9. WINDOW_UPDATE
      - [x] 1: Sends a WINDOW_UPDATE frame with stream ID 0
      - [x] 2: Sends a WINDOW_UPDATE frame with stream ID 1
    - 3.10. CONTINUATION
      - [x] 1: Sends a CONTINUATION frame
      - [x] 2: Sends multiple CONTINUATION frames