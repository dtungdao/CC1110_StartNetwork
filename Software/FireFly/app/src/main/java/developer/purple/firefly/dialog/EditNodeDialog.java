package developer.purple.firefly.dialog;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.text.InputFilter;
import android.text.InputType;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.NumberPicker;
import android.widget.Toast;

import developer.purple.firefly.R;
import developer.purple.firefly.model.NodeModel;
import developer.purple.firefly.utils.CheckValidationUtils;

/**
 * Created by tnluan on 4/13/16.
 */
public class EditNodeDialog extends Dialog implements View.OnClickListener, CompoundButton.OnCheckedChangeListener {
    private Context mContext;
    private NodeModel mNodeModel;

    private EditText nodeAddrEdt;
    private CheckBox[] cbOxArr = new CheckBox[16];
    private NumberPicker[] numPickers = new NumberPicker[16];
    private Button btnCheckAll, btnClearAll, btnConfirm, btnCancel;

    private OnConfirmListener mOnConfirmListener;

    private String[] mCountDownValues = new String[61];

    public static final float MIN_VALUE = 0.5f; // min value of time
    public static final float MAX_VALUE = 30f; // max value of time
    public static final float STEP = 0.5f; // each step value when scrolling number picker

    public interface OnConfirmListener {
        void onConfirm();
        boolean onCheckDuplicate(String nodeAdrr);
    }

    public void setOnConfirmListener(OnConfirmListener listener) {
        mOnConfirmListener = listener;
    }

    public EditNodeDialog(Context context, NodeModel nodeModel) {
        super(context);
        this.mContext = context;
        this.mNodeModel = nodeModel;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.dialog_edit_node);

        initNecessaryData();
        findViews();
        setupViews();
    }

    /**
     * Init display values for number picker, from 0.5 to 30
     */
    private void initNecessaryData() {
        mCountDownValues[0] = "None";
        Float step = MIN_VALUE;
        for (int i = 1; i < 61; i++) {
            if (i % 2 == 0) {
                mCountDownValues[i] = String.valueOf(step.intValue());
            } else {
                mCountDownValues[i] = String.valueOf(step);
            }
            step += STEP;
        }
    }

    private void findViews() {
        nodeAddrEdt = (EditText) findViewById(R.id.edtNodeAddr);

        cbOxArr[0]  = (CheckBox) findViewById(R.id.checkBox01);
        cbOxArr[1]  = (CheckBox) findViewById(R.id.checkBox02);
        cbOxArr[2]  = (CheckBox) findViewById(R.id.checkBox03);
        cbOxArr[3]  = (CheckBox) findViewById(R.id.checkBox04);
        cbOxArr[4]  = (CheckBox) findViewById(R.id.checkBox05);
        cbOxArr[5]  = (CheckBox) findViewById(R.id.checkBox06);
        cbOxArr[6]  = (CheckBox) findViewById(R.id.checkBox07);
        cbOxArr[7]  = (CheckBox) findViewById(R.id.checkBox08);
        cbOxArr[8]  = (CheckBox) findViewById(R.id.checkBox09);
        cbOxArr[9]  = (CheckBox) findViewById(R.id.checkBox10);
        cbOxArr[10] = (CheckBox) findViewById(R.id.checkBox11);
        cbOxArr[11] = (CheckBox) findViewById(R.id.checkBox12);
        cbOxArr[12] = (CheckBox) findViewById(R.id.checkBox13);
        cbOxArr[13] = (CheckBox) findViewById(R.id.checkBox14);
        cbOxArr[14] = (CheckBox) findViewById(R.id.checkBox15);
        cbOxArr[15] = (CheckBox) findViewById(R.id.checkBox16);

        numPickers[0]  = (NumberPicker) findViewById(R.id.numPicker01);
        numPickers[1]  = (NumberPicker) findViewById(R.id.numPicker02);
        numPickers[2]  = (NumberPicker) findViewById(R.id.numPicker03);
        numPickers[3]  = (NumberPicker) findViewById(R.id.numPicker04);
        numPickers[4]  = (NumberPicker) findViewById(R.id.numPicker05);
        numPickers[5]  = (NumberPicker) findViewById(R.id.numPicker06);
        numPickers[6]  = (NumberPicker) findViewById(R.id.numPicker07);
        numPickers[7]  = (NumberPicker) findViewById(R.id.numPicker08);
        numPickers[8]  = (NumberPicker) findViewById(R.id.numPicker09);
        numPickers[9]  = (NumberPicker) findViewById(R.id.numPicker10);
        numPickers[10] = (NumberPicker) findViewById(R.id.numPicker11);
        numPickers[11] = (NumberPicker) findViewById(R.id.numPicker12);
        numPickers[12] = (NumberPicker) findViewById(R.id.numPicker13);
        numPickers[13] = (NumberPicker) findViewById(R.id.numPicker14);
        numPickers[14] = (NumberPicker) findViewById(R.id.numPicker15);
        numPickers[15] = (NumberPicker) findViewById(R.id.numPicker16);

        btnCheckAll = (Button) findViewById(R.id.btnCheckAll);
        btnClearAll = (Button) findViewById(R.id.btnClearAll);
        btnConfirm = (Button) findViewById(R.id.btnConfirm);
        btnCancel = (Button) findViewById(R.id.btnCancel);
    }

    private void setupViews() {
        nodeAddrEdt.setFilters(new InputFilter[]{new InputFilter.AllCaps()});
        nodeAddrEdt.setText(mNodeModel.getNodeAddr());
        nodeAddrEdt.setSelection(mNodeModel.getNodeAddr().length());

        for (int i = 0; i < numPickers.length; i++) {
            numPickers[i].setMinValue(0);
            numPickers[i].setMaxValue(60);
            numPickers[i].setValue(0);
            // Get the latest time value that user has set before clicking Confirm button
            // and then display it whenever showing dialog
            if (mNodeModel.getTimes()[i] == 0) {
                numPickers[i].setValue(0);
            } else {
                for (int j = 1; j < mCountDownValues.length; j++) {
                    if (Float.valueOf(mCountDownValues[j]) == mNodeModel.getTimes()[i]) {
                        numPickers[i].setValue(j);
                    }
                }
            }
            numPickers[i].setDisplayedValues(mCountDownValues);
            numPickers[i].setEnabled(false);
        }

        // Get the latest checking status that user has set
        // and then display it whenever showing dialog
        for (int i = 0; i < cbOxArr.length; i++) {
            cbOxArr[i].setOnCheckedChangeListener(this);
            cbOxArr[i].setChecked(mNodeModel.getLEDs()[i]);
        }

        btnCheckAll.setOnClickListener(this);
        btnClearAll.setOnClickListener(this);
        btnConfirm.setOnClickListener(this);
        btnCancel.setOnClickListener(this);
    }

    /**
     * When user clicks Confirm button, collect all count down times, and store in Node object
     */
    private void getAllCountDowns() {
        for (int i = 0; i < numPickers.length; i++) {
            if (numPickers[i].getValue() == 0) {
                mNodeModel.getTimes()[i] = 0;
            } else {
                mNodeModel.getTimes()[i] = Float.valueOf(numPickers[i].getDisplayedValues()[numPickers[i].getValue()]);
            }
        }
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        switch (buttonView.getId()) {
            case R.id.checkBox01:
                numPickers[0].setEnabled(isChecked);
                mNodeModel.getLEDs()[0] = isChecked; // store checking status in Node object
                break;
            case R.id.checkBox02:
                numPickers[1].setEnabled(isChecked);
                mNodeModel.getLEDs()[1] = isChecked;
                break;
            case R.id.checkBox03:
                numPickers[2].setEnabled(isChecked);
                mNodeModel.getLEDs()[2] = isChecked;
                break;
            case R.id.checkBox04:
                numPickers[3].setEnabled(isChecked);
                mNodeModel.getLEDs()[3] = isChecked;
                break;
            case R.id.checkBox05:
                numPickers[4].setEnabled(isChecked);
                mNodeModel.getLEDs()[4] = isChecked;
                break;
            case R.id.checkBox06:
                numPickers[5].setEnabled(isChecked);
                mNodeModel.getLEDs()[5] = isChecked;
                break;
            case R.id.checkBox07:
                numPickers[6].setEnabled(isChecked);
                mNodeModel.getLEDs()[6] = isChecked;
                break;
            case R.id.checkBox08:
                numPickers[7].setEnabled(isChecked);
                mNodeModel.getLEDs()[7] = isChecked;
                break;
            case R.id.checkBox09:
                numPickers[8].setEnabled(isChecked);
                mNodeModel.getLEDs()[8] = isChecked;
                break;
            case R.id.checkBox10:
                numPickers[9].setEnabled(isChecked);
                mNodeModel.getLEDs()[9] = isChecked;
                break;
            case R.id.checkBox11:
                numPickers[10].setEnabled(isChecked);
                mNodeModel.getLEDs()[10] = isChecked;
                break;
            case R.id.checkBox12:
                numPickers[11].setEnabled(isChecked);
                mNodeModel.getLEDs()[11] = isChecked;
                break;
            case R.id.checkBox13:
                numPickers[12].setEnabled(isChecked);
                mNodeModel.getLEDs()[12] = isChecked;
                break;
            case R.id.checkBox14:
                numPickers[13].setEnabled(isChecked);
                mNodeModel.getLEDs()[13] = isChecked;
                break;
            case R.id.checkBox15:
                numPickers[14].setEnabled(isChecked);
                mNodeModel.getLEDs()[14] = isChecked;
                break;
            case R.id.checkBox16:
                numPickers[15].setEnabled(isChecked);
                mNodeModel.getLEDs()[15] = isChecked;
                break;
        }
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btnCheckAll:
                for (CheckBox cboxArrItem : cbOxArr) {
                    cboxArrItem.setChecked(true);
                }
                break;
            case R.id.btnClearAll:
                for (CheckBox cboxArrItem : cbOxArr) {
                    cboxArrItem.setChecked(false);
                }
                break;
            case R.id.btnConfirm:
                if (!CheckValidationUtils.isNodeAddrValid(mContext, nodeAddrEdt.getText().toString())) {
                    break;
                }
                if (!mNodeModel.getNodeAddr().equalsIgnoreCase(nodeAddrEdt.getText().toString())) {
                    if (mOnConfirmListener != null) {
                        if (mOnConfirmListener.onCheckDuplicate(nodeAddrEdt.getText().toString())) {
                            Toast.makeText(mContext, "This node address exists already", Toast.LENGTH_SHORT).show();
                            break;
                        }
                    }
                    AlertDialog.Builder alertDialog = new AlertDialog.Builder(mContext);
                    alertDialog.setMessage("Do you want to change Node Address?");
                    alertDialog.setNegativeButton("Cancel", null);
                    alertDialog.setPositiveButton("OK", new OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            mNodeModel.setNodeAddr(nodeAddrEdt.getText().toString());
                            getAllCountDowns();
                            if (mOnConfirmListener != null) {
                                mOnConfirmListener.onConfirm();
                            }
                            dismiss();
                        }
                    });
                    alertDialog.show();
                } else {
                    mNodeModel.setNodeAddr(nodeAddrEdt.getText().toString());
                    getAllCountDowns();
                    if (mOnConfirmListener != null) {
                        mOnConfirmListener.onConfirm();
                    }
                    dismiss();
                }
                break;
            case R.id.btnCancel:
                dismiss();
                break;
            default:
                break;
        }
    }
}