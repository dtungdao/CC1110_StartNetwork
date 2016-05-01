package developer.purple.firefly.view;

import android.app.Activity;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.os.AsyncTask;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.RadioButton;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.Socket;
import java.util.Timer;
import java.util.TimerTask;

import developer.purple.firefly.R;
import developer.purple.firefly.dialog.EditNodeDialog;
import developer.purple.firefly.model.NodeModel;
import developer.purple.firefly.utils.Utils;

/**
 * Created by luantran on 1/13/16.
 */
public class NodeView extends RelativeLayout {
    private Context mContext;
    private LayoutInflater mInflater;
    private RelativeLayout mMainLayout;
    private TextView mNodeAddrTv;
    private NodeModel mNodeModel;

    private String mHexNumber;
    private boolean mIsActive;

    private Timer mSendTCPTimer;

    private float mCountTime = 0;

    private OnDropListener mOnDropListener;

    public interface OnDropListener {
        void onDrop(int left, int top);
    }

    public void setOnDropListener(OnDropListener listener) {
        mOnDropListener = listener;
    }

    public void updatePosition(int left, int top) {
        if (mOnDropListener != null) {
            mOnDropListener.onDrop(left, top);
        }
    }

    public NodeView(Context context, NodeModel nodeModel) {
        super(context);
        mContext = context;
        mInflater = LayoutInflater.from(context);
        mNodeModel = nodeModel;
        mHexNumber = nodeModel.getNodeAddr(); // hex number
        mIsActive = nodeModel.isActive(); // if isActive is true, set background color as green, otherwise, gray

        setup();
    }

    public void setup() {
        View v = mInflater.inflate(R.layout.item_node, this, true);

        mMainLayout = (RelativeLayout) v.findViewById(R.id.rlMain);
        mMainLayout.setBackgroundResource(mIsActive ? R.drawable.node_on_shape : R.drawable.node_off_shape);

        mNodeAddrTv = (TextView) v.findViewById(R.id.tvNodeAddr);
        mNodeAddrTv.setText(mHexNumber);
    }

    public NodeModel getNodeInfo() {
        return mNodeModel;
    }

    /**
     * Call this action after clicking Confirm button on EditNodeDialog
     */
    public void startTimerToSendTCP() {
        // If user clicks Confirm button continuously while the previous timer does not finish, stop this timer
        if (mSendTCPTimer != null) {
            mCountTime = 0;
            mSendTCPTimer.cancel();
        }
        ((Activity) mContext).runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mMainLayout.setBackgroundResource(R.drawable.node_on_shape);
                mNodeAddrTv.setText(mNodeModel.getNodeAddr());
            }
        });

        // Send the first data frame
        sendTCP();

        // Init a timer and a count time from 0, this timer will call a task each 0.5s
        // This task will increase the count time up 0.5
        // Travel all of LEDs, if the count time == count down time which has been set, turn off that LED
        // and then send new data frame
        mSendTCPTimer = new Timer();
        mSendTCPTimer.schedule(new TimerTask() {
            @Override
            public void run() {
                mCountTime += EditNodeDialog.STEP;
                boolean allBoxUnchecked = true;

                for (int i = 0; i < mNodeModel.getLEDs().length; i++) {
                    if (mNodeModel.getLEDs()[i] && mNodeModel.getTimes()[i] == mCountTime) {
                        mNodeModel.getLEDs()[i] = false;
                        sendTCP();
                    }
                    if (mNodeModel.getLEDs()[i]) {
                        allBoxUnchecked = false;
                    }
                }

                if (allBoxUnchecked || mCountTime == EditNodeDialog.MAX_VALUE) {
                    if (allBoxUnchecked) {
                        ((Activity) mContext).runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                mMainLayout.setBackgroundResource(R.drawable.node_off_shape);
                            }
                        });
                    }
                    mCountTime = 0;
                    mSendTCPTimer.cancel();
                }
            }
        }, 500, 500);
    }

    /**
     * Call this function in case that user wanna turn on all LEDs
     */
    public void turnOn() {
        for (int i = 0; i < mNodeModel.getLEDs().length; i++) {
            mNodeModel.getLEDs()[i] = true;
            mNodeModel.getTimes()[i] = 0;
        }
        startTimerToSendTCP();
    }

    /**
     * Call this function in case that user wanna send broadcast TCP to turn off all LEDs
     */
    public void turnOff() {
        ((Activity) mContext).runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mMainLayout.setBackgroundResource(R.drawable.node_off_shape);
            }
        });
        mCountTime = 0;
        if (mSendTCPTimer != null) {
            mSendTCPTimer.cancel();
        }
        mNodeModel.setLEDs(new boolean[16]);
    }

    private void sendTCP() {
        // Get node address
        String nodeAddr = mNodeModel.getNodeAddr();

        // Get LED status
        String ledsSta = getCheckBoxVal();

        // Forming the data frame
        String strData = ":x" + nodeAddr + " " + ledsSta + "X";

        Log.d("IVC", strData);
        new TCPClientTask().execute(strData);
    }

    public void sendBroadcastTCP() {
        // Get node address
        String nodeAddr = "FF";

        // Get LED status
        String ledsSta = "0000";

        // Forming the data frame
        String strData = ":x" + nodeAddr + " " + ledsSta + "X";

        Log.d("IVC", strData);
        new TCPClientTask().execute(strData);
    }

    public String getCheckBoxVal() {
        short val = 0;

        for(int idx = 0; idx < mNodeModel.getLEDs().length; idx++) {
            val <<= 1;
            if (mNodeModel.getLEDs()[idx]) {
                val = (short)(val | 1);
            }
        }

        return String.format("%04X", val);
    }

    private class TCPClientTask extends AsyncTask<String, Void, String> {
        private final int TCP_PORT = 8888;
        private final String SERVER_IP = "192.168.4.1"; // esp8266

        private Socket tcpClient;

        private InputStream inStream;
        private OutputStream outStream;
        private byte[] inArr = new byte[1024];
        private String errString = "";

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
        }

        @Override
        protected String doInBackground(String... params) {
            try {
                // Connect to TCP server
                tcpClient = new Socket(InetAddress.getByName(SERVER_IP), TCP_PORT);

                // Initialized input/output buffer
                inStream = tcpClient.getInputStream();
                outStream = tcpClient.getOutputStream();

                // Receiver data, block here until get data
                //inStream.read(inArr);
                //strData = new String(inArr);

                // Send data
                outStream.write(params[0].getBytes("UTF-8"));

                // Update UI
                publishProgress();

                // Close connection
                tcpClient.close();
            } catch (Exception e) {
                e.printStackTrace();
                errString = e.toString();
            }
            return params[0];
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            super.onProgressUpdate(values);
        }

        @Override
        protected void onPostExecute(String result) {
            super.onPostExecute(result);
            if (!errString.isEmpty()) {
                Utils.writeToFile(result + "\n" + errString);
                Toast.makeText(mContext, errString, Toast.LENGTH_SHORT).show();
            } else {
                Utils.writeToFile(result + "\n" + "SUCCESS");
            }
        }
    }
}
