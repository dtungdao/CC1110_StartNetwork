package developer.purple.firefly.dialog;

import android.app.Activity;
import android.app.Dialog;
import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.Toast;

import developer.purple.firefly.R;

/**
 * Created by dkan10 on 4/5/16.
 */
public class CustomConfirmDialog extends Dialog implements View.OnClickListener {
    public Activity activity;
    public Dialog dialog;
    public Button btnJustOne, btnArrange, btnCancel;

    public CustomConfirmDialog(Activity activity) {
        super(activity);
        this.activity = activity;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.custom_dialog_add_new_node);
        btnJustOne = (Button) findViewById(R.id.btnJustOne);
        btnArrange = (Button) findViewById(R.id.btnArrange);
        btnCancel  = (Button) findViewById(R.id.btnCancel);
        btnJustOne.setOnClickListener(this);
        btnArrange.setOnClickListener(this);
        btnCancel.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btnJustOne:
                Toast.makeText(getContext(), "btnJustOne", Toast.LENGTH_SHORT).show();
                break;
            case R.id.btnArrange:
                Toast.makeText(getContext(), "btnArrange", Toast.LENGTH_SHORT).show();
                break;
            case R.id.btnCancel:
                Toast.makeText(getContext(), "btnCancel", Toast.LENGTH_SHORT).show();
                break;
            default:
                break;
        }
        dismiss();
    }
}