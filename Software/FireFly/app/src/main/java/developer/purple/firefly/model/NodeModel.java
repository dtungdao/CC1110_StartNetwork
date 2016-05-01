package developer.purple.firefly.model;

import android.graphics.Point;

/**
 * Created by tnluan on 4/6/16.
 */
public class NodeModel {
    private String mNodeAddr;
    private boolean mIsActive;
    private Point mPointInLayout;

    // LEDs status
    private boolean[] mLEDs = new boolean[16];
    private float[] mTimes = new float[16];

    public NodeModel(String nodeAddr, boolean isActive) {
        mNodeAddr = nodeAddr;
        mIsActive = isActive;
        mPointInLayout = new Point();
        mPointInLayout.x = 20;
        mPointInLayout.y = 20;
    }

    public NodeModel(String nodeAddr, boolean isActive, Point pointInLayout) {
        mNodeAddr = nodeAddr;
        mIsActive = isActive;
        mPointInLayout = pointInLayout;
    }

    public String getNodeAddr() {
        return mNodeAddr;
    }

    public void setNodeAddr(String mNodeAddr) {
        this.mNodeAddr = mNodeAddr;
    }

    public boolean isActive() {
        return mIsActive;
    }

    public void setActive(boolean mIsActive) {
        this.mIsActive = mIsActive;
    }

    public Point getPointInLayout() {
        return mPointInLayout;
    }

    public void setPointInLayout(Point mPointInLayout) {
        this.mPointInLayout = mPointInLayout;
    }

    public boolean[] getLEDs() {
        return mLEDs;
    }

    public void setLEDs(boolean[] mLEDs) {
        this.mLEDs = mLEDs;
    }

    public float[] getTimes() {
        return mTimes;
    }

    public void setTimes(float[] mTimes) {
        this.mTimes = mTimes;
    }
}
