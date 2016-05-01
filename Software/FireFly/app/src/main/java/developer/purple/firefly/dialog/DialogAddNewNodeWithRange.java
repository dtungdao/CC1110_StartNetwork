package developer.purple.firefly.dialog;

import android.app.Dialog;
import android.content.Context;
import android.graphics.Point;
import android.os.Bundle;
import android.text.InputFilter;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.Random;

import developer.purple.firefly.R;
import developer.purple.firefly.common.Constant;
import developer.purple.firefly.model.NodeModel;
import developer.purple.firefly.utils.CheckValidationUtils;
import developer.purple.firefly.utils.Utils;

/**
 * Created by dkan10 on 4/6/16.
 */
public class DialogAddNewNodeWithRange extends Dialog implements
        View.OnClickListener {

    public Context activity;
    public Dialog dialog;
    public Button btnOk, btnCancel;
    public EditText edtNodeArrangeFrom, edtNumberOfNode;
    ArrayList<String> hexaArrayList;
    private OnConfirmListener mOnConfirmListener;

    public interface OnConfirmListener {
        void addNode(NodeModel nodeModel);
        boolean onCheckDuplicate(String nodeAdrr);
    }

    public void setOnConfirmListener(OnConfirmListener listener) {
        mOnConfirmListener = listener;
    }

    public DialogAddNewNodeWithRange(Context activity) {
        super(activity);
        this.activity = activity;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.dialog_add_new_node_with_range);

        edtNodeArrangeFrom = (EditText) findViewById(R.id.edtNodeArrangeFrom);
        edtNodeArrangeFrom.setFilters(new InputFilter[]{new InputFilter.AllCaps()});
        edtNumberOfNode = (EditText) findViewById(R.id.edtNumberOfNode);

        btnOk = (Button) findViewById(R.id.btnOk);
        btnCancel = (Button) findViewById(R.id.btnCancel);

        btnOk.setOnClickListener(this);
        btnCancel.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btnOk:
                String nodeValueFrom = edtNodeArrangeFrom.getText().toString();
                if (!CheckValidationUtils.isNodeAddrValid(activity, nodeValueFrom)) {
                    break;
                }
                if (edtNumberOfNode.getText().toString().isEmpty()) {
                    Toast.makeText(getContext(), "Please input number of node", Toast.LENGTH_SHORT).show();
                    break;
                }
                int numberOfNode = Integer.parseInt(edtNumberOfNode.getText().toString());

//                if (!hexaArrayList.contains(nodeValueFrom)) {
//                    Toast.makeText(getContext(), "Node Name Incorrect!!!", Toast.LENGTH_SHORT).show();
//                    break;
//                }
                hexaArrayList = Constant.getHexadecimal();
                int indexOfValueFrom = getCategoryPos(hexaArrayList, nodeValueFrom);

                for (int i = 0 ;i < numberOfNode; i++) {
                    if (indexOfValueFrom + i >= hexaArrayList.size()) {
                        break;
                    }
                    if (mOnConfirmListener != null) {
                        if (!mOnConfirmListener.onCheckDuplicate(hexaArrayList.get(indexOfValueFrom + i))) {
                            Point pointInScreen = new Point();
                            pointInScreen.x = 10 + (new Random().nextInt(Utils.getScreenWidth(activity))*2/3 - 10);
                            pointInScreen.y = 10 + (new Random().nextInt(Utils.getScreenHeight(activity))*2/3 - 10);
                            NodeModel nodeModel = new NodeModel(hexaArrayList.get(indexOfValueFrom + i), false, pointInScreen);
                            if (mOnConfirmListener != null) {
                                mOnConfirmListener.addNode(nodeModel);
                            }
                        }
                    }

                }
                dismiss();
                break;

            case R.id.btnCancel:
                Toast.makeText(getContext(), "btnCancel", Toast.LENGTH_SHORT).show();
                dismiss();
                break;
            default:
                break;
        }
    }

    private int getCategoryPos(ArrayList<String> arrayList, String category) {
        return arrayList.indexOf(category);
    }
}