package org.quietmodem.Quiet;

public class FrameTransmitter extends BaseFrameTransmitter {
    @Override
    protected void initSystem() throws ModemException {
        this.quietSystem.initOpenSL();
    }

    public FrameTransmitter(FrameTransmitterConfig conf) throws ModemException {
        super(conf);
    }
}
