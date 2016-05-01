package developer.purple.firefly;

import android.app.Activity;
import android.graphics.Point;
import android.os.Bundle;

import android.os.Handler;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.List;

import developer.purple.firefly.dialog.EditNodeDialog;
import developer.purple.firefly.manager.DataManager;
import developer.purple.firefly.manager.DragController;
import developer.purple.firefly.model.NodeModel;
import developer.purple.firefly.model.SetItem;
import developer.purple.firefly.view.DragLayer;
import developer.purple.firefly.view.NodeView;

/**
 * This activity presents two images and a text view and allows them to be dragged around.
 * Press and hold on a view initiates a drag. 
 * After clicking the Short button, dragging starts with a short touch rather than a long touch.
 *
 * <p> This activity is derviced from the Android Launcher class.
 * 
 */
public class NodeManagerActivity extends Activity implements View.OnLongClickListener, View.OnClickListener, View.OnTouchListener {
    private DragController mDragController;   // Object that sends out drag-drop events while a view is being moved.
    private DragLayer mDragLayer;             // The ViewGroup that supports drag-drop.
    private boolean mLongClickStartsDrag = false;    // If true, it takes a long click to start the drag operation.
                                                    // Otherwise, any touch event starts a drag.

    private static final int CHANGE_TOUCH_MODE_MENU_ID = Menu.FIRST;
    private static final int TURN_ON_ALL_ID = Menu.NONE;
    private static final int TURN_OFF_ALL_ID = Menu.FIRST;

    public static final boolean Debugging = false;

    private static final int LONG_CLICK_RESPONSE_TIME = 200; // It's used to distinguish between
                                                            // click event and long click event in onTouch listener
    private long mCurrentTime = 0; // It gets current time in milliseconds
    private List<NodeModel> mNodeList;
    private List<NodeView> mNodeViewList;

    /**
     * onCreate - called when the activity is first created.
     *
     * Creates a drag controller and sets up three views so click and long click on the views are sent to this activity.
     * The onLongClick method starts a drag sequence.
     *
     */

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mDragController = new DragController(this);

        setContentView(R.layout.activity_node_manager);
        setupViews();
    }

    /**
     * Build a menu for the activity.
     *
     */
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);

//        menu.add(0, CHANGE_TOUCH_MODE_MENU_ID, 0, "Change Touch Mode");
        menu.add(0, TURN_ON_ALL_ID, 0, "Turn On All");
        menu.add(0, TURN_OFF_ALL_ID, 0, "Turn Off All");
        return true;
    }

    /**
     * Handle a click on a view. Tell the user to use a long click (press).
     *
     */
    public void onClick(View v) {
        if (mLongClickStartsDrag) {
           // Tell the user that it takes a long click to start dragging.
//           toast("Press and hold to drag an image.");
        }
    }

    /**
     * Handle a long click.
     * If mLongClick only is true, this will be the only way to start a drag operation.
     *
     * @param v View
     * @return boolean - true indicates that the event was handled
     */
    public boolean onLongClick(View v) {
        if (mLongClickStartsDrag) {
            //trace ("onLongClick in view: " + v + " touchMode: " + v.isInTouchMode ());

            // Make sure the drag was started by a long press as opposed to a long click.
            // (Note: I got this from the Workspace object in the Android Launcher code.
            //  I think it is here to ensure that the device is still in touch mode as we start the drag operation.)
            if (!v.isInTouchMode()) {
               toast("isInTouchMode returned false. Try touching the view again.");
               return false;
            }
            return startDrag(v);
        }

        // If we get here, return false to indicate that we have not taken care of the event.
        return false;
    }

    /**
     * Perform an action in response to a menu item being clicked.
     *
     */
    public boolean onOptionsItemSelected(MenuItem item) {
        switch(item.getItemId()) {
//            case CHANGE_TOUCH_MODE_MENU_ID:
//                mLongClickStartsDrag = !mLongClickStartsDrag;
//                String message = mLongClickStartsDrag ? "Changed touch mode. Drag now starts on long touch (click)."
//                                                      : "Changed touch mode. Drag now starts on touch (click).";
//                Toast.makeText(getApplicationContext(), message, Toast.LENGTH_LONG).show();
//                return true;
            case TURN_ON_ALL_ID:
                // Stop all timer
                for (NodeView nodeView : mNodeViewList) {
                    nodeView.turnOn();
                }
                return true;
            case TURN_OFF_ALL_ID:
                // Stop all timer
                for (NodeView nodeView : mNodeViewList) {
                    nodeView.turnOff();
                }
                // Send broadcast TCP
                if (mNodeViewList.size() > 0) {
                    mNodeViewList.get(0).sendBroadcastTCP();
                }
                return true;
        }
        return super.onOptionsItemSelected(item);
    }

    /**
     * Resume the activity.
     */

    @Override
    protected void onResume() {
        super.onResume();
    }

    /**
     * This is the starting point for a drag operation if mLongClickStartsDrag is false.
     * It looks for the down event that gets generated when a user touches the screen.
     * Only that initiates the drag-drop sequence.
     *
     */
    public boolean onTouch(final View v, MotionEvent ev) {
        // If we are configured to start only on a long click, we are not going to handle any events here.
        if (mLongClickStartsDrag) return false;

//        final boolean handledHere = false;

        final int action = ev.getAction();

        // In the situation where a long click is not needed to initiate a drag, simply start on the down event.
        if (action == MotionEvent.ACTION_DOWN) {
            if (mCurrentTime == 0) {
                mCurrentTime = System.currentTimeMillis();
            }
            // Use handler to execute tasks after LONG_CLICK_RESPONSE_TIME
            // If user does not leave touch during this time, start drag. Otherwise, not start drag.
            new Handler().postDelayed(new Runnable() {
                @Override
                public void run() {
                    NodeManagerActivity.this.runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            if (mCurrentTime != 0) {
                                boolean handledHere = startDrag(v);
                                if (handledHere) v.performClick();
                                mCurrentTime = 0;
                            }
                        }
                    });
                }
            }, LONG_CLICK_RESPONSE_TIME);
        } else if (action == MotionEvent.ACTION_UP) {
            // This logic is same as click event
            if (System.currentTimeMillis() - mCurrentTime < LONG_CLICK_RESPONSE_TIME) {
                mCurrentTime = 0;
                EditNodeDialog cdd = new EditNodeDialog(NodeManagerActivity.this, ((NodeView) v).getNodeInfo());
                cdd.setOnConfirmListener(new EditNodeDialog.OnConfirmListener() {
                    @Override
                    public void onConfirm() {
                        ((NodeView) v).startTimerToSendTCP();
                    }

                    @Override
                    public boolean onCheckDuplicate(String nodeAdrr) {
                        for (NodeModel nodeModel : mNodeList) {
                            if (nodeModel.getNodeAddr().equalsIgnoreCase(nodeAdrr)) {
                                return true;
                            }
                        }
                        return false;
                    }
                });
                cdd.show();
            }
        }

        return false;
    }

    /**
     * Start dragging a view.
     *
     */

    public boolean startDrag(View v) {
        // Let the DragController initiate a drag-drop sequence.
        // I use the dragInfo to pass along the object being dragged.
        // I'm not sure how the Launcher designers do this.
        Object dragInfo = v;
        mDragController.startDrag(v, mDragLayer, dragInfo, DragController.DRAG_ACTION_MOVE);
        return true;
    }

    /**
     * Finds all the views we need and configure them to send click events to the activity.
     *
     */
    private void setupViews() {
        DragController dragController = mDragController;

        mDragLayer = (DragLayer) findViewById(R.id.drag_layer);
        mDragLayer.setDragController(dragController);
        dragController.addDropTarget(mDragLayer);

        SetItem userModel = DataManager.getSet(getIntent().getIntExtra("position", 0));
        mNodeList = userModel.getNodeModels();
        mNodeViewList = new ArrayList<>();

        for (final NodeModel nodeModel : mNodeList) {
            ViewGroup.LayoutParams layoutParams = new ViewGroup.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);

            NodeView nodeView = new NodeView(this, nodeModel);
            nodeView.setLayoutParams(layoutParams);
            // Set the latest position
            nodeView.setX(nodeModel.getPointInLayout().x);
            nodeView.setY(nodeModel.getPointInLayout().y);
            nodeView.setOnClickListener(this);
            nodeView.setOnLongClickListener(this);
            nodeView.setOnTouchListener(this);
            nodeView.setOnDropListener(new NodeView.OnDropListener() {
                @Override
                public void onDrop(int left, int top) {
                    Point pointInLayout = nodeModel.getPointInLayout();
                    pointInLayout.set(left, top);
                }
            });

            mDragLayer.addView(nodeView);
            mNodeViewList.add(nodeView);
        }
    }

    /**
     * Show a string on the screen via Toast.
     *
     * @param msg String
     * @return void
     */
    public void toast(String msg) {
        Toast.makeText(getApplicationContext(), msg, Toast.LENGTH_SHORT).show();
    } // end toast

    /**
     * Send a message to the debug log and display it using Toast.
     */
    public void trace(String msg) {
        if (!Debugging) return;
        Log.d("NodeManagerActivity", msg);
        toast(msg);
    }
} // end class
