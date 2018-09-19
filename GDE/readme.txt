Testing
   * Install virtual loopback COM port driver: http://com0com.sourceforge.net

Communication protocol
    Serial protocol using 9600 bauds, 8 bits per transferred byte, odd parity, one stop bit, no handshake nor flow control.
    Lines should be terminated by CR character i.e. '\n', or ASCII 0x0A.

    > marks data inbound to the Emulator, < marks outbound data

    Messages sent by the emulator have following format:
        ID Dx/y Bx Mx Lx lx message
            ID is some 32 bit number, uniquely identifying the message sent by the emulator
            Dx/y door status
                x is one of: (C)losed, (O)pen, 1 to X - denotes last detected magnetic stripe
                y denotes count of all magnetic stripes
            Bx blockage present: 0 no blockage, 1 blockage present
            Mx motor status: (U)p, (D)own, (S)stopped
            Lx indoor lights status: 0 off, 1 on
            lx outdoor lights status: 0 off, 1 on
            message: status, optionally with most recent change

    Acknowledge messages sent to the emulator have format:
        A ID
            Acknowledge outbound message with given ID

    Commands sent to the emulator have following format:
        ID X
            ID is some 32 bit number greater than zero which identifies the command sent to the emulator
            X denotes command, one of:
                C fully close
                c close to next magnetic stripe
                O fully open
                o open to next magnetic stripe
                S immediately stop the motor
                L0 switch off the indoor lights
                L1 switch on the indoor lights
                l0 switch off the outdoor lights
                l1 switch on the outdoor lights

Example communication
    < 1 DC/10 B0 MS L0 l0 OK
    > A1
    < 2 D1/10 B0 MU L0 l0 Opening 10%
    > A2
    < 3 D2/10 B0 MU L0 l1 Opening 20%, outdoor lights on
    > A3
    < 4 DO/10 B0 MS L0 l1 Fully opened
    > A4
    > 1 C
    < 5 DO/10 B0 MD L0 l1 Closing 99%
    > A5
    < 6 D9/10 B0 MD L0 l1 Closing 90%
    > A6
    > 2 S
    < 7 D9/10 B0 MS L0 l1 Motor stopped at 90%
    > A7
    > 3 C
    < 8 D9/10 B0 MD L0 l1 Closing 90%
    > A8
    < 9 D8/10 B0 MS L0 l1 Motor stopped at 80%
    > A9
    > 4 C
    < 10 D8/10 B0 MD L0 l1 Closing 80%
    > A10
    < 11 D7/10 B0 MD L0 l1 Closing 70%
    > A11
    < 12 D7/10 B1 MS L0 l1 Motor stopped at 70% on blockage
    > A12
    ...
