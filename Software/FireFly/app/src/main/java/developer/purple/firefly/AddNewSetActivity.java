package developer.purple.firefly;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.List;

import developer.purple.firefly.adapter.NewNodeAdapter;
import developer.purple.firefly.dialog.DialogAddNewNodeJustOne;
import developer.purple.firefly.dialog.DialogAddNewNodeWithRange;
import developer.purple.firefly.dialog.DialogAddNode;
import developer.purple.firefly.manager.DataManager;
import developer.purple.firefly.model.NodeModel;
import developer.purple.firefly.model.SetItem;

/**
 * Created by dkan10 on 4/5/16.
 */
public class AddNewSetActivity extends Activity implements View.OnClickListener {

    private Button btnAddNewNode, btnNewNodeConfirm, btnNewNodeCancel;
    private ListView lvNode;
    private EditText edtSetName;
    private List<NodeModel> nodeModelList;
    private NewNodeAdapter newNodeAdapter;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_new_set);

        edtSetName = (EditText) findViewById(R.id.edtSetName);
        lvNode = (ListView) findViewById(R.id.lvNode);
        btnAddNewNode = (Button) findViewById(R.id.btnAddNode);
        btnNewNodeCancel = (Button) findViewById(R.id.btnNewnodeCancel);
        btnNewNodeConfirm = (Button) findViewById(R.id.btnNewNodeConfirm);

        btnNewNodeCancel.setOnClickListener(this);
        btnNewNodeConfirm.setOnClickListener(this);
        btnAddNewNode.setOnClickListener(this);
        btnAddNewNode.setOnClickListener(this);

        nodeModelList = new ArrayList<>();
        newNodeAdapter = new NewNodeAdapter(getApplicationContext(), R.layout.item_new_node_list_view, nodeModelList);
        lvNode.setAdapter(newNodeAdapter);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {

            case R.id.btnAddNode:
                DialogAddNode dialogAddNewNode = new DialogAddNode(AddNewSetActivity.this);
                dialogAddNewNode.setOnConfirmListenerJustOne(new DialogAddNewNodeJustOne.OnConfirmListener() {
                    @Override
                    public void addNode(NodeModel nodeModel) {
                        nodeModelList.add(nodeModel);
                        newNodeAdapter.notifyDataSetChanged();
                    }

                    @Override
                    public boolean onCheckDuplicate(String nodeAdrr) {
                        for (NodeModel nodeModel : nodeModelList) {
                            if (nodeModel.getNodeAddr().equalsIgnoreCase(nodeAdrr)) {
                                return true;
                            }
                        }
                        return false;
                    }
                });
                dialogAddNewNode.setOnConfirmListenerWithRange(new DialogAddNewNodeWithRange.OnConfirmListener() {
                    @Override
                    public void addNode(NodeModel nodeModel) {
                        nodeModelList.add(nodeModel);
                        newNodeAdapter.notifyDataSetChanged();
                    }

                    @Override
                    public boolean onCheckDuplicate(String nodeAdrr) {
                        for (NodeModel nodeModel : nodeModelList) {
                            if (nodeModel.getNodeAddr().equalsIgnoreCase(nodeAdrr)) {
                                return true;
                            }
                        }
                        return false;
                    }
                });
                dialogAddNewNode.show();
                break;

            case R.id.btnNewNodeConfirm:
                String setName = edtSetName.getText().toString().trim();
                if (setName.isEmpty() || setName.length() == 0 || setName.equals("") || setName == null) {
                    Toast.makeText(getApplicationContext(), "Please enter Set Name", Toast.LENGTH_SHORT).show();
                }
                // check item exist.
                else {
                    SetItem setItem = new SetItem();
                    setItem.setName(setName);
                    setItem.setNodeModels(nodeModelList);
                    DataManager.addNewSet(setItem);
                    finish();
                }
                break;

            case R.id.btnNewnodeCancel:
                finish();
                break;

            default:
                break;
        }

    }
}
