// $Id: test.java,v 1.14 2002-08-15 16:14:06 recampb Exp $
import java.io.*;
import javax.swing.*;
import java.awt.event.*;
import java.awt.*;
import javax.swing.tree.*;
import javax.swing.event.*;

public class test extends JPanel implements ActionListener, ChangeListener {

  // looks like an unnamed static function
  static {
	  System.loadLibrary("testClass");
	}
  
  public native void InitCpp();  
  public native void Print();
  public native void SetMinSc(int x);
  public native int GetMinSc();
  public native void SetScope(String s);
  public native void Interact();
  public native void CommandLine();
  public native void DoCommand(String command, String arguments);
  //public native void Rotate(int x, int y , int z);

 
  long cpp_obj;
  long console;
  long consoleObjects;

  int newNodeSuffix = 1;
  protected JButton testButton, b2, flipBookButton, nextTimeButton, prevTimeButton, selectTimeButton, addButton, removeButton, clearButton;
  protected JTextField minScalarTF, selectTimeTF, panXTF, panYTF, rotateXTF, rotateYTF, rotateZTF, zoomTF, selectTimeTFF, panXTFF, panYTFF, rotateXTFF, rotateYTFF,  rotateZTFF, zoomTFF;
  protected JPanel leftPanel, leftRootPanel, leftFramePanel, leftBodyPanel;
  protected JScrollPane leftRootScrollPane, leftFrameScrollPane, leftBodyScrollPane;
  protected DynamicTree treePanel;
  protected JTabbedPane tabbedPane;
  protected JSlider selectTimeSlider, selectTimeSliderF;
  protected JSplitPane /* splitPane, */ splitPane2;
  private boolean tabBool;
  protected JFrame parentFrame;

  public test(JFrame parentFrame){
    this.parentFrame = parentFrame;
    InitCpp();
    Print();
    tabbedPane = new JTabbedPane();
    tabbedPane.addChangeListener(this);
    tabBool = false;

    final JPanel rootPanel = new JPanel();
    final JPanel framePanel = new JPanel();
    final JPanel bodyPanel = new JPanel();
    JPanel bodyVarPanel = new JPanel();

    JMenuBar menuBar = new JMenuBar();
    JMenu file = new JMenu("File");
    JMenu help = new JMenu("Help");
    JMenuItem exitItem = new JMenuItem("Exit");
    exitItem.addActionListener(this);
    exitItem.setActionCommand("Exit");
    JMenuItem addBodyItem = new JMenuItem("Add Body");
    JCheckBoxMenuItem frameNumsItem = new JCheckBoxMenuItem("Frame Numbers", false);
    JMenuItem removeBodyItem = new JMenuItem("Remove Body");
    JMenuItem saveItem = new JMenuItem("Save");
    JMenuItem saveFlipItem = new JMenuItem("Save Flip Book");
    JMenuItem windowSizeItem= new JMenuItem("Window Size");

    GridBagConstraints gbc = new GridBagConstraints();    
    gbc.anchor=gbc.NORTHEAST;
    
    file.add(addBodyItem);
    file.add(removeBodyItem);
    file.add(frameNumsItem);
    file.add(saveItem);
    file.add(saveFlipItem);
    file.add(windowSizeItem);
    file.add(exitItem);

    menuBar.add(file);
    menuBar.add(help);


//     setLayout(new GridBagLayout());
   
//     add(menuBar, new GridBagConstraints(0,0,4,1,0.0,0.05, GridBagConstraints.NORTHWEST, GridBagConstraints.HORIZONTAL, new Insets(0,0,0,0),0,0));
    
//     add(tabbedPane,new GridBagConstraints(0,1,4,1,0.0,0.0,GridBagConstraints.NORTHWEST,GridBagConstraints.HORIZONTAL,new Insets(0,0,0,0),0,0));
    setLayout(new BorderLayout());
    add(menuBar, BorderLayout.NORTH);
    add(tabbedPane, BorderLayout.CENTER);
  
    
    //bodyVarPanel.setLayout(new GridLayout(2,2,20,5));


    minScalarTF = new JTextField(Integer.toString(GetMinSc()), 8);
    JLabel minScalarL = new JLabel("min_Scalar_Range");
    JButton bodyVarOKButton = new JButton("OK");
    JButton bodyVarCancelButton = new JButton("Cancel");
    bodyVarOKButton.addActionListener(this);
    bodyVarOKButton.setActionCommand("BodyVarOK");

    bodyVarPanel.setLayout(new GridBagLayout()); 
    gbc.gridx=0; gbc.gridy=0; gbc.gridwidth=1;
    bodyVarPanel.add(minScalarL, gbc);
    gbc.gridx=1; gbc.gridy=0; gbc.gridwidth=1;
    bodyVarPanel.add(minScalarTF, gbc);

    JPanel buttonPanel=new JPanel();
    buttonPanel.add(bodyVarOKButton);
    buttonPanel.add(bodyVarCancelButton);
    gbc.gridx=0; gbc.gridy=1; gbc.gridwidth=2; gbc.anchor=gbc.NORTHEAST;
    bodyVarPanel.add(buttonPanel,gbc);

    JRadioButton ShowContoursButton = new JRadioButton("ShowContours", false);
    JRadioButton HideContoursButton = new JRadioButton ("HideContours", true);
    JRadioButton ShowCuttingButton = new JRadioButton ("ShowCuttingPlane", false);
    JRadioButton HideCuttingButton = new JRadioButton ("HideCuttingPlane", true);

    ShowContoursButton.addActionListener(this);
    HideContoursButton.addActionListener(this);    
    ShowCuttingButton.addActionListener(this);
    HideCuttingButton.addActionListener(this);

    ShowContoursButton.setActionCommand("ShowContours");
    HideContoursButton.setActionCommand("HideContours");
    ShowCuttingButton.setActionCommand("ShowCutting");
    HideCuttingButton.setActionCommand("HideCutting");

    
    
    tabbedPane.addTab("Root Commands", rootPanel);
    tabbedPane.addTab("Frame Commands", framePanel);
    tabbedPane.addTab("Body Commands", bodyPanel);
    tabbedPane.addTab("Body Variables", bodyVarPanel);

    
    /** rootPanel stuff  **/
    rootPanel.setLayout(new GridLayout(1,2));
    
    JPanel rootLeftPanel = new JPanel();
    JScrollPane rootLeftScroll = new JScrollPane(rootLeftPanel);
    rootLeftPanel.setLayout(new GridBagLayout());
    JPanel rootRightPanel = new JPanel();
    JScrollPane rootRightScroll = new JScrollPane(rootRightPanel);
    rootRightPanel.setLayout(new GridBagLayout());
    gbc.gridx=0; gbc.gridy=0; gbc.gridwidth=1;  
    rootPanel.add(rootLeftScroll);

    gbc.gridx=1; gbc.gridy=0; gbc.gridwidth=1; 
    rootPanel.add(rootRightScroll);

    testButton = new JButton ("Exit");
    testButton.addActionListener(this);
    testButton.setActionCommand("Exit");
    gbc.gridx=1; gbc.gridy=4; gbc.gridwidth=1;
    rootRightPanel.add(testButton, gbc);

    JButton commandButton = new JButton("Command Line");
    commandButton.addActionListener(this);
    commandButton.setActionCommand("CommandLine");
    gbc.gridx=2;
    rootRightPanel.add(commandButton, gbc);


    flipBookButton = new JButton("Flip Book");
    flipBookButton.addActionListener(this);
    flipBookButton.setActionCommand("FlipBook");
    gbc.gridx=3; gbc.gridy=3; gbc.gridwidth=1;
    rootRightPanel.add(flipBookButton, gbc);
    
    nextTimeButton = new JButton("Next Time Step");
    nextTimeButton.addActionListener(this);
    nextTimeButton.setActionCommand("NextTimeStep");
    gbc.gridx=2; gbc.gridy=3; gbc.gridwidth=1;
    rootRightPanel.add(nextTimeButton, gbc);

    prevTimeButton = new JButton("Previous Time Step");
    prevTimeButton.addActionListener(this);
    prevTimeButton.setActionCommand("PreviousTimeStep");
    gbc.gridx=1; gbc.gridy=3; gbc.gridwidth=1;
    rootRightPanel.add(prevTimeButton, gbc);   
    
    selectTimeSlider = new JSlider(0,10,0);
    selectTimeSlider.addChangeListener(this);
    JLabel selectTimeLabel = new JLabel("Time Step");

    gbc.gridx=0; gbc.gridy=2; gbc.gridwidth=1;
    rootRightPanel.add(selectTimeLabel, gbc);

    selectTimeTF = new JTextField("0", 5);
    selectTimeTF.setEnabled(true);
    gbc.gridx=1; gbc.gridy=2; gbc.gridwidth=2; gbc.anchor = gbc.CENTER;
    rootRightPanel.add(selectTimeSlider, gbc);
    gbc.gridx=3; gbc.gridy=2; gbc.gridwidth=1; gbc.anchor = gbc.NORTHEAST;
    rootRightPanel.add(selectTimeTF, gbc);
  
    
    JButton panButton = new JButton("Pan");
    panButton.addActionListener(this);
    panButton.setActionCommand("Pan");
    
    JButton rotateButton = new JButton("Rotate");
    rotateButton.addActionListener(this);
    rotateButton.setActionCommand("Rotate");
    
    JButton zoomButton = new JButton("Zoom");
    zoomButton.addActionListener(this);
    zoomButton.setActionCommand("Zoom");

    JLabel rotXLabel = new JLabel("X");
    JLabel rotYLabel = new JLabel("Y");
    JLabel rotZLabel = new JLabel("Z");

    JLabel panXLabel = new JLabel("X");
    JLabel panYLabel = new JLabel("Y");
    JLabel zoomFactorLabel = new JLabel("Factor");

    panXTF = new JTextField("0", 5);
    panYTF = new JTextField("0", 5);
    rotateXTF = new JTextField("0", 5);
    rotateYTF = new JTextField("0", 5);
    rotateZTF = new JTextField("0", 5);
    zoomTF = new JTextField("0", 5);

    JButton resetViewButton = new JButton("Reset View");
    JButton interactiveButton = new JButton("Interactive");
    JButton updateButton = new JButton("Update");
    interactiveButton.addActionListener(this);
    interactiveButton.setActionCommand("Interact");
    resetViewButton.addActionListener(this);
    resetViewButton.setActionCommand("ResetView");
  

    gbc.anchor = gbc.NORTHEAST;
    gbc.insets=new Insets(4,4,4,4);
    gbc.gridx=1; gbc.gridy=3; gbc.gridwidth=1;
    rootLeftPanel.add(rotateButton, gbc);
    gbc.gridx=0; gbc.gridy=4; gbc.gridwidth=1;
    rootLeftPanel.add(rotXLabel, gbc);
    gbc.gridx=1; gbc.gridy=4; gbc.gridwidth=1;
    rootLeftPanel.add(rotateXTF, gbc);
    gbc.gridx=0; gbc.gridy=5; gbc.gridwidth=1;
    rootLeftPanel.add(rotYLabel, gbc);
    gbc.gridx=1; gbc.gridy=5; gbc.gridwidth=1;
    rootLeftPanel.add(rotateYTF, gbc);
    gbc.gridx=0; gbc.gridy=6; gbc.gridwidth=1;
    rootLeftPanel.add(rotZLabel, gbc);
    gbc.gridx=1; gbc.gridy=6; gbc.gridwidth=1;
    rootLeftPanel.add(rotateZTF, gbc);

    rootLeftPanel.add(new JSeparator(JSeparator.HORIZONTAL),new GridBagConstraints(0,7,2,1,0.0,0.0,GridBagConstraints.CENTER,GridBagConstraints.HORIZONTAL,new Insets(0,0,0,0),0,0));
    gbc.gridx=1; gbc.gridy=8; gbc.gridwidth=1;
    rootLeftPanel.add(panButton, gbc);
    gbc.gridx=0; gbc.gridy=9; gbc.gridwidth=1;
    rootLeftPanel.add(panXLabel, gbc);
    gbc.gridx=1; gbc.gridy=9; gbc.gridwidth=1;
    rootLeftPanel.add(panXTF, gbc);
    gbc.gridx=0; gbc.gridy=10; gbc.gridwidth=1;
    rootLeftPanel.add(panYLabel, gbc);
    gbc.gridx=1; gbc.gridy=10; gbc.gridwidth=1;
    rootLeftPanel.add(panYTF, gbc);
    
	rootLeftPanel.add(new JSeparator(JSeparator.HORIZONTAL),new GridBagConstraints(0,11,2,1,0.0,0.0,GridBagConstraints.CENTER,GridBagConstraints.HORIZONTAL,new Insets(0,0,0,0),0,0));
	
	gbc.gridx=1; gbc.gridy = 12;
	rootLeftPanel.add(zoomButton, gbc);
	gbc.gridx=0; gbc.gridy = 13;
	rootLeftPanel.add(zoomFactorLabel, gbc);
	gbc.gridx=1; gbc.gridy = 13;
	rootLeftPanel.add(zoomTF, gbc);

	rootLeftPanel.add(new JSeparator(JSeparator.HORIZONTAL),new GridBagConstraints(0,14,2,1,0.0,0.0,GridBagConstraints.CENTER,GridBagConstraints.HORIZONTAL,new Insets(0,0,0,0),0,0));
	
	gbc.gridx=0; gbc.gridy = 15; gbc.gridwidth=2;
	rootLeftPanel.add(resetViewButton, gbc);
	gbc.gridx=0; gbc.gridy = 16;
	rootLeftPanel.add(interactiveButton, gbc);
	gbc.gridx=0; gbc.gridy = 17;
	rootLeftPanel.add(updateButton, gbc);

	/** end root panel **/





    /* frame panel stuff */
    framePanel.setLayout(new GridLayout(1,2));
    
    JPanel frameLeftPanel = new JPanel();
    JScrollPane frameLeftScroll = new JScrollPane(frameLeftPanel);
    frameLeftPanel.setLayout(new GridBagLayout());
    JPanel frameRightPanel = new JPanel();
    JScrollPane frameRightScroll = new JScrollPane(frameRightPanel);
    frameRightPanel.setLayout(new GridBagLayout()); 
    framePanel.add(frameLeftScroll);
     
    framePanel.add(frameRightScroll);
    


    JButton flipBookButtonF = new JButton("Flip Book");
    flipBookButtonF.addActionListener(this);
    flipBookButtonF.setActionCommand("FlipBook");


    JButton nextTimeButtonF = new JButton("Next Time Step");
    nextTimeButtonF.addActionListener(this);
    nextTimeButtonF.setActionCommand("NextTimeStep");


    JButton prevTimeButtonF = new JButton("Previous Time Step");
    prevTimeButtonF.addActionListener(this);
    prevTimeButtonF.setActionCommand("PreviousTimeStep");


    selectTimeSliderF = new JSlider(0,10,0);
    selectTimeSliderF.addChangeListener(this);
    JLabel selectTimeLabelF = new JLabel("Time Step");
    JButton selectTimeButtonF = new JButton("Select Time Step");
    selectTimeButtonF.addActionListener(this);
    selectTimeButtonF.setActionCommand("SelectTimeStep");


    selectTimeTFF = new JTextField("0", 5);
    selectTimeTFF.setEnabled(true);
    selectTimeTFF.addActionListener(this);



    JButton panButtonF = new JButton("Pan");
    panButtonF.addActionListener(this);
    panButtonF.setActionCommand("Pan");
    
    JButton rotateButtonF = new JButton("Rotate");
    rotateButtonF.addActionListener(this);
    rotateButtonF.setActionCommand("Rotate");
    
    JButton zoomButtonF = new JButton("Zoom");
    zoomButtonF.addActionListener(this);
    zoomButtonF.setActionCommand("Zoom");
    
    JLabel rotXLabelF = new JLabel("X");
    JLabel rotYLabelF = new JLabel("Y");
    JLabel rotZLabelF = new JLabel("Z");
    
    JLabel panXLabelF = new JLabel("X");
    JLabel panYLabelF = new JLabel("Y");
    JLabel zoomFactorLabelF = new JLabel("Factor");
    
    panXTFF = new JTextField("0", 5);
    panYTFF = new JTextField("0", 5);
    rotateXTFF = new JTextField("0", 5);
    rotateYTFF = new JTextField("0", 5);
    rotateZTFF = new JTextField("0", 5);
    zoomTFF = new JTextField("0", 5);
    
    JButton resetViewButtonF = new JButton("Reset View");
    JButton interactiveButtonF = new JButton("Interactive");
    JButton updateButtonF = new JButton("Update");
    
    JLabel bgLabel = new JLabel("Background Color");
    JRadioButton bgBlackButton = new JRadioButton("Black");
    bgBlackButton.addActionListener(this);
    bgBlackButton.setActionCommand("Black BG");
    bgBlackButton.setSelected(true);
    JRadioButton bgWhiteButton = new JRadioButton("White");
    bgWhiteButton.addActionListener(this);
    bgWhiteButton.setActionCommand("White BG");
    ButtonGroup bgButtonGroup = new ButtonGroup();
    bgButtonGroup.add(bgBlackButton);
    bgButtonGroup.add(bgWhiteButton);
    
    JLabel repLabel = new JLabel("Representation");
    JRadioButton repSurfButton = new JRadioButton("Surface");
    repSurfButton.setSelected(true);
    repSurfButton.addActionListener(this);
    repSurfButton.setActionCommand("Surf");
    JRadioButton repWireButton = new JRadioButton("Wire");
    repWireButton.addActionListener(this);
    repWireButton.setActionCommand("Wire");
    JRadioButton repPointButton = new JRadioButton("Points");
    repPointButton.addActionListener(this);
    repPointButton.setActionCommand("Points");
    ButtonGroup repButtons = new ButtonGroup();
    repButtons.add(repSurfButton);
    repButtons.add(repWireButton);
    repButtons.add(repPointButton);
    
    JLabel axesLabel = new JLabel("Axes");
    JLabel colorBarLabel = new JLabel ("Color Bar");
    JLabel nodeNumsLabel = new JLabel ("Node Numbers");
    JLabel elemNumsLabel = new JLabel ("Element Numbers");
    JRadioButton axesShowButton = new JRadioButton("Show");
    JRadioButton axesHideButton = new JRadioButton("Hide");
    ButtonGroup axesButtons = new ButtonGroup();
    JRadioButton colorBarShowButton = new JRadioButton("Show");
    JRadioButton colorBarHideButton = new JRadioButton("Hide");
    ButtonGroup colorBarButtons = new ButtonGroup();
    JRadioButton nodeNumsShowButton = new JRadioButton("Show");
    JRadioButton nodeNumsHideButton = new JRadioButton("Hide");
    ButtonGroup nodeNumsButtons = new ButtonGroup();
    JRadioButton elemNumsShowButton = new JRadioButton("Show");
    JRadioButton elemNumsHideButton = new JRadioButton("Hide");
    ButtonGroup elemNumsButtons = new ButtonGroup();
    axesButtons.add(axesShowButton);
    axesButtons.add(axesHideButton);
    colorBarButtons.add(colorBarShowButton);
    colorBarButtons.add(colorBarHideButton);
    nodeNumsButtons.add(nodeNumsShowButton);
    nodeNumsButtons.add(nodeNumsHideButton);
    elemNumsButtons.add(elemNumsShowButton);
    elemNumsButtons.add(elemNumsHideButton);
    axesHideButton.setSelected(true);
    colorBarHideButton.setSelected(true);
    nodeNumsHideButton.setSelected(true);
    elemNumsHideButton.setSelected(true);
    
    axesShowButton.addActionListener(this);
    axesHideButton.addActionListener(this);
    axesShowButton.setActionCommand("Show Axes");
    axesHideButton.setActionCommand("Hide Axes");
    colorBarShowButton.addActionListener(this);
    colorBarHideButton.addActionListener(this);
    colorBarShowButton.setActionCommand("Show Color Bar");
    colorBarHideButton.setActionCommand("Hide Color Bar");
    nodeNumsShowButton.addActionListener(this);
    nodeNumsHideButton.addActionListener(this);
    nodeNumsShowButton.setActionCommand("Show Node Nums");
    nodeNumsHideButton.setActionCommand("Hide Node Nums");
    elemNumsShowButton.addActionListener(this);
    elemNumsHideButton.addActionListener(this);
    elemNumsShowButton.setActionCommand("Show Elem Nums");
    elemNumsHideButton.setActionCommand("Hide Elem Nums");
    

    gbc.anchor = gbc.NORTHEAST;
    gbc.insets=new Insets(4,4,4,4);
	frameLeftPanel.setLayout(new GridBagLayout());
	gbc.gridx=1; gbc.gridy=0; gbc.gridwidth=1;
	frameLeftPanel.add(rotateButtonF, gbc);
	gbc.gridx=0; gbc.gridy=1; gbc.gridwidth=1;
	frameLeftPanel.add(rotXLabelF, gbc);
	gbc.gridx=1; gbc.gridy=1; gbc.gridwidth=1;
	frameLeftPanel.add(rotateXTFF, gbc);
	gbc.gridx=0; gbc.gridy=2; gbc.gridwidth=1;
	frameLeftPanel.add(rotYLabelF, gbc);
	gbc.gridx=1; gbc.gridy=2; gbc.gridwidth=1;
	frameLeftPanel.add(rotateYTFF, gbc);
	gbc.gridx=0; gbc.gridy=3; gbc.gridwidth=1;
	frameLeftPanel.add(rotZLabelF, gbc);
	gbc.gridx=1; gbc.gridy=3; gbc.gridwidth=1;
	frameLeftPanel.add(rotateZTFF, gbc);

	frameLeftPanel.add(new JSeparator(JSeparator.HORIZONTAL),new GridBagConstraints(0,4,2,1,0.0,0.0,GridBagConstraints.CENTER,GridBagConstraints.HORIZONTAL,new Insets(0,0,0,0),0,0));
	gbc.gridx=1; gbc.gridy=5; gbc.gridwidth=1;
	frameLeftPanel.add(panButtonF, gbc);
	gbc.gridx=0; gbc.gridy=6; gbc.gridwidth=1;
	frameLeftPanel.add(panXLabelF, gbc);
	gbc.gridx=1; gbc.gridy=6; gbc.gridwidth=1;
	frameLeftPanel.add(panXTFF, gbc);
	gbc.gridx=0; gbc.gridy=7; gbc.gridwidth=1;
	frameLeftPanel.add(panYLabelF, gbc);
	gbc.gridx=1; gbc.gridy=7; gbc.gridwidth=1;
	frameLeftPanel.add(panYTFF, gbc);

	frameLeftPanel.add(new JSeparator(JSeparator.HORIZONTAL),new GridBagConstraints(0,8,2,1,0.0,0.0,GridBagConstraints.CENTER,GridBagConstraints.HORIZONTAL,new Insets(0,0,0,0),0,0));
	
	gbc.gridx=1; gbc.gridy = 9;
	frameLeftPanel.add(zoomButtonF, gbc);
	gbc.gridx=0; gbc.gridy = 10;
	frameLeftPanel.add(zoomFactorLabelF, gbc);
	gbc.gridx=1; gbc.gridy = 10;
	frameLeftPanel.add(zoomTFF, gbc);

	frameLeftPanel.add(new JSeparator(JSeparator.HORIZONTAL),new GridBagConstraints(0,11,2,1,0.0,0.0,GridBagConstraints.CENTER,GridBagConstraints.HORIZONTAL,new Insets(0,0,0,0),0,0));
	
	gbc.gridx=0; gbc.gridy = 12; gbc.gridwidth=2;
	frameLeftPanel.add(resetViewButtonF, gbc);
	gbc.gridx=0; gbc.gridy = 13;

	frameLeftPanel.add(interactiveButtonF, gbc);
	gbc.gridx=0; gbc.gridy = 14;
	frameLeftPanel.add(updateButtonF, gbc);
	gbc.gridx=0; gbc.gridy = 0; gbc.gridwidth=1; gbc.anchor = gbc.NORTHWEST;
	frameRightPanel.add(bgLabel, gbc);
	gbc.gridx=1; gbc.gridy = 0;
	frameRightPanel.add(bgBlackButton, gbc);
	gbc.gridx=2; gbc.gridy = 0;
	frameRightPanel.add(bgWhiteButton, gbc);
	gbc.gridx=0; gbc.gridy = 1;
	frameRightPanel.add(repLabel, gbc);
	gbc.gridx=1; gbc.gridy = 1;
	frameRightPanel.add(repSurfButton, gbc);
	gbc.gridx=2; gbc.gridy = 1;
	frameRightPanel.add(repWireButton, gbc);
	gbc.gridx=1; gbc.gridy = 2;
	frameRightPanel.add(repPointButton, gbc);
// 	gbc.gridx=0; gbc.gridy = 20;
// 	leftFramePanel.add(axesLabel, gbc);
// 	gbc.gridx=0; gbc.gridy = 21;
// 	leftFramePanel.add(axesShowButton, gbc);
// 	gbc.gridx=1; gbc.gridy = 21;
// 	leftFramePanel.add(axesHideButton, gbc);
// 	gbc.gridx=0; gbc.gridy = 22;
// 	leftFramePanel.add(colorBarLabel, gbc);
// 	gbc.gridx=0; gbc.gridy = 23;
// 	leftFramePanel.add(colorBarShowButton, gbc);
// 	gbc.gridx=1; gbc.gridy = 23;
// 	leftFramePanel.add(colorBarHideButton, gbc);
// 	gbc.gridx=0; gbc.gridy = 24;
// 	leftFramePanel.add(nodeNumsLabel, gbc);
// 	gbc.gridx=0; gbc.gridy = 25;
// 	leftFramePanel.add(nodeNumsShowButton, gbc);
// 	gbc.gridx=1; gbc.gridy = 25;
// 	leftFramePanel.add(nodeNumsHideButton, gbc);
// 	gbc.gridx=0; gbc.gridy = 26;
// 	leftFramePanel.add(elemNumsLabel, gbc);
// 	gbc.gridx=0; gbc.gridy = 27;
// 	leftFramePanel.add(elemNumsShowButton, gbc);
// 	gbc.gridx=1; gbc.gridy = 27;
// 	leftFramePanel.add(elemNumsHideButton, gbc); 





    treePanel = new DynamicTree();
    populateTree(treePanel);

    JPanel rightPanel = new JPanel();
    rightPanel.setLayout(new GridLayout(1,2));

    addButton = new JButton("Add");
    addButton.addActionListener(this);
    addButton.setActionCommand("AddBody");

    removeButton = new JButton("Remove");
    removeButton.addActionListener(this);
    removeButton.setActionCommand("RemoveBody");
    
//     JButton clearButton = new JButton("Clear");
//     clearButton.addActionListener(this);
//     clearButton.setActionCommand("Clear");

    rightPanel.add(addButton);
    rightPanel.add(removeButton);
    //   rightPanel.add(clearButton);
    
    gbc.gridx=0; gbc.gridy=3; gbc.gridheight=1; gbc.gridwidth=1;
    treePanel.add(rightPanel, gbc);
    add(treePanel, BorderLayout.EAST);
    





    //treePanel.setup();
    treePanel.getTree().addTreeSelectionListener(new TreeSelectionListener() {
                public void valueChanged(TreeSelectionEvent e) {
                    DefaultMutableTreeNode node = (DefaultMutableTreeNode)
                                       treePanel.getTree().getLastSelectedPathComponent();
                    
                    if (node == null) return;

                    Object nodeInfo = node.getUserObject();
		    if (((String)nodeInfo).equals( "0.body")){
		      tabbedPane.setSelectedComponent(bodyPanel);
		      SetScope((String)nodeInfo);

		    }
		    else if (((String)nodeInfo).equals("Console Root")){
		      tabbedPane.setSelectedComponent(rootPanel);
		      //remove(leftPanel);
		      //add(leftRootPanel,new GridBagConstraints(0,2,1,1,0.05,1.0,GridBagConstraints.NORTHWEST,GridBagConstraints.BOTH,new Insets(0,0,0,0),0,0));
		      //splitPane.setLeftComponent(leftRootPanel);
		      SetScope((String)nodeInfo);
		      updateUI();

		    }
		    else if (((String)nodeInfo).equals("0.0.frame")){
		      tabbedPane.setSelectedComponent(framePanel);
		      //remove(leftPanel);
		      //add(leftRootPanel,new GridBagConstraints(0,2,1,1,0.05,1.0,GridBagConstraints.NORTHWEST,GridBagConstraints.BOTH,new Insets(0,0,0,0),0,0));
		      //splitPane.setLeftComponent(leftFrameScrollPane);
		      SetScope((String)nodeInfo);
		      updateUI();

		    }

		    System.out.println((String)nodeInfo);
		    
		}

                
    }); 
      



	/* left body panel stuff */
      
	JButton interactiveButtonB = new JButton("Interactive");
	JButton updateButtonB = new JButton("Update");
	JButton cuttingButtonB = new JButton("Show Cutting Plane");
	interactiveButtonB.addActionListener(this);
	interactiveButtonB.setActionCommand("Interactive");
	updateButtonB.addActionListener(this);
	updateButtonB.setActionCommand("Update");
	cuttingButtonB.addActionListener(this);
	cuttingButtonB.setActionCommand("ShowCutting");

	JLabel repLabelB = new JLabel("Representation");
	JRadioButton repSurfButtonB = new JRadioButton("Surface");
	repSurfButtonB.setSelected(true);
	repSurfButtonB.addActionListener(this);
	repSurfButtonB.setActionCommand("Surf");
	JRadioButton repWireButtonB = new JRadioButton("Wire");
	repWireButtonB.addActionListener(this);
	repWireButtonB.setActionCommand("Wire");
	JRadioButton repPointButtonB = new JRadioButton("Points");
	repPointButtonB.addActionListener(this);
	repPointButtonB.setActionCommand("Points");
	ButtonGroup repButtonsB = new ButtonGroup();
	repButtonsB.add(repSurfButton);
	repButtonsB.add(repWireButton);
	repButtonsB.add(repPointButton);

	JLabel nodeNumsLabelB = new JLabel ("Node Numbers");
	JLabel elemNumsLabelB = new JLabel ("Element Numbers");

	JRadioButton nodeNumsShowButtonB = new JRadioButton("Show");
	JRadioButton nodeNumsHideButtonB = new JRadioButton("Hide");
	ButtonGroup nodeNumsButtonsB = new ButtonGroup();
	JRadioButton elemNumsShowButtonB = new JRadioButton("Show");
	JRadioButton elemNumsHideButtonB = new JRadioButton("Hide");
	ButtonGroup elemNumsButtonsB = new ButtonGroup();

	nodeNumsButtonsB.add(nodeNumsShowButtonB);
	nodeNumsButtonsB.add(nodeNumsHideButtonB);
	elemNumsButtonsB.add(elemNumsShowButtonB);
	elemNumsButtonsB.add(elemNumsHideButtonB);

	nodeNumsHideButtonB.setSelected(true);
	elemNumsHideButtonB.setSelected(true);

	nodeNumsShowButtonB.addActionListener(this);
	nodeNumsHideButtonB.addActionListener(this);
	nodeNumsShowButtonB.setActionCommand("Show Node Nums");
	nodeNumsHideButtonB.setActionCommand("Hide Node Nums");
	elemNumsShowButtonB.addActionListener(this);
	elemNumsHideButtonB.addActionListener(this);
	elemNumsShowButtonB.setActionCommand("Show Elem Nums");
	elemNumsHideButtonB.setActionCommand("Hide Elem Nums");


	JLabel contoursLabelB = new JLabel ("Contours");
	JLabel glyphsLabelB = new JLabel ("Glyphs");

	JRadioButton contoursShowButtonB = new JRadioButton("Show");
	JRadioButton contoursHideButtonB = new JRadioButton("Hide");
	ButtonGroup contoursButtonsB = new ButtonGroup();
	JRadioButton glyphsShowButtonB = new JRadioButton("Show");
	JRadioButton glyphsHideButtonB = new JRadioButton("Hide");
	ButtonGroup glyphsButtonsB = new ButtonGroup();

	contoursButtonsB.add(contoursShowButtonB);
	contoursButtonsB.add(contoursHideButtonB);
	glyphsButtonsB.add(glyphsShowButtonB);
	glyphsButtonsB.add(glyphsHideButtonB);

	contoursHideButtonB.setSelected(true);
	glyphsHideButtonB.setSelected(true);

	contoursShowButtonB.addActionListener(this);
	contoursHideButtonB.addActionListener(this);
	contoursShowButtonB.setActionCommand("Show Contours");
	contoursHideButtonB.setActionCommand("Hide Contours");
	glyphsShowButtonB.addActionListener(this);
	glyphsHideButtonB.addActionListener(this);
	glyphsShowButtonB.setActionCommand("Show Glyphs");
	glyphsHideButtonB.setActionCommand("Hide Glyphs");


	leftBodyPanel = new JPanel();
	leftBodyScrollPane = new JScrollPane(leftBodyPanel);
	gbc.anchor = gbc.NORTHEAST;
	gbc.insets=new Insets(4,4,4,4);
	leftBodyPanel.setLayout(new GridBagLayout());
 	gbc.gridx=1; gbc.gridy=0; gbc.gridwidth=1;
 	leftBodyPanel.add(interactiveButtonB, gbc);
 	gbc.gridx=1; gbc.gridy=1; gbc.gridwidth=1;
 	leftBodyPanel.add(updateButtonB, gbc);
	gbc.gridx=0; gbc.gridy=2; gbc.gridwidth=1;gbc.anchor = gbc.NORTHWEST;
	leftBodyPanel.add(repLabelB, gbc);
 	gbc.gridx=0; gbc.gridy=3; gbc.gridwidth=1;
 	leftBodyPanel.add(repSurfButtonB, gbc);
 	gbc.gridx=1; gbc.gridy=3; gbc.gridwidth=1;
 	leftBodyPanel.add(repWireButtonB, gbc);
 	gbc.gridx=0; gbc.gridy=4; gbc.gridwidth=1;
 	leftBodyPanel.add(repPointButtonB, gbc);
 	gbc.gridx=0; gbc.gridy=5; gbc.gridwidth=1;
 	leftBodyPanel.add(nodeNumsLabelB, gbc);

// 	leftBodyPanel.add(new JSeparator(JSeparator.HORIZONTAL),new GridBagConstraints(0,4,2,1,0.0,0.0,GridBagConstraints.CENTER,GridBagConstraints.HORIZONTAL,new Insets(0,0,0,0),0,0));
	gbc.gridx=0; gbc.gridy=6; gbc.gridwidth=1;
 	leftBodyPanel.add(nodeNumsShowButtonB, gbc);
 	gbc.gridx=1; gbc.gridy=6; gbc.gridwidth=1;
 	leftBodyPanel.add(nodeNumsHideButtonB, gbc);
 	gbc.gridx=0; gbc.gridy=7; gbc.gridwidth=1;
 	leftBodyPanel.add(elemNumsLabelB, gbc);
 	gbc.gridx=0; gbc.gridy=8; gbc.gridwidth=1;
 	leftBodyPanel.add(elemNumsShowButtonB, gbc);
 	gbc.gridx=1; gbc.gridy=8; gbc.gridwidth=1;
 	leftBodyPanel.add(elemNumsHideButtonB, gbc);

// 	leftBodyPanel.add(new JSeparator(JSeparator.HORIZONTAL),new GridBagConstraints(0,8,2,1,0.0,0.0,GridBagConstraints.CENTER,GridBagConstraints.HORIZONTAL,new Insets(0,0,0,0),0,0));
	
 	gbc.gridx=0; gbc.gridy = 9;
 	leftBodyPanel.add(contoursLabelB, gbc);
 	gbc.gridx=0; gbc.gridy = 10;
 	leftBodyPanel.add(contoursShowButtonB, gbc);
 	gbc.gridx=1; gbc.gridy = 10;
 	leftBodyPanel.add(contoursHideButtonB, gbc);

// 	leftFramePanel.add(new JSeparator(JSeparator.HORIZONTAL),new GridBagConstraints(0,11,2,1,0.0,0.0,GridBagConstraints.CENTER,GridBagConstraints.HORIZONTAL,new Insets(0,0,0,0),0,0));
	
// 	gbc.gridx=0; gbc.gridy = 12; gbc.gridwidth=2;
// 	leftFramePanel.add(resetViewButtonF, gbc);
// // 	gbc.gridx=0; gbc.gridy = 13;
// // 	leftFramePanel.add(addButtonF, gbc);
// // 	gbc.gridx=0; gbc.gridy = 14;
// // 	leftFramePanel.add(removeButtonF, gbc);
// // 	gbc.gridx=0; gbc.gridy = 15;
// // 	leftFramePanel.add(clearButtonF, gbc);
// 	gbc.gridx=0; gbc.gridy = 13;
// 	leftFramePanel.add(interactiveButtonF, gbc);
// 	gbc.gridx=0; gbc.gridy = 14;
// 	leftFramePanel.add(updateButtonF, gbc);
// 	gbc.gridx=0; gbc.gridy = 15; gbc.gridwidth=1; gbc.anchor = gbc.NORTHWEST;
// 	leftFramePanel.add(bgLabel, gbc);
// 	gbc.gridx=0; gbc.gridy = 16;
// 	leftFramePanel.add(bgBlackButton, gbc);
// 	gbc.gridx=1; gbc.gridy = 16;
// 	leftFramePanel.add(bgWhiteButton, gbc);
// 	gbc.gridx=0; gbc.gridy = 17;
// 	leftFramePanel.add(repLabel, gbc);
// 	gbc.gridx=0; gbc.gridy = 18;
// 	leftFramePanel.add(repSurfButton, gbc);
// 	gbc.gridx=1; gbc.gridy = 18;
// 	leftFramePanel.add(repWireButton, gbc);
// 	gbc.gridx=0; gbc.gridy = 19;
// 	leftFramePanel.add(repPointButton, gbc);
// 	gbc.gridx=0; gbc.gridy = 20;
// 	leftFramePanel.add(axesLabel, gbc);
// 	gbc.gridx=0; gbc.gridy = 21;
// 	leftFramePanel.add(axesShowButton, gbc);
// 	gbc.gridx=1; gbc.gridy = 21;
// 	leftFramePanel.add(axesHideButton, gbc);
// 	gbc.gridx=0; gbc.gridy = 22;
// 	leftFramePanel.add(colorBarLabel, gbc);
// 	gbc.gridx=0; gbc.gridy = 23;
// 	leftFramePanel.add(colorBarShowButton, gbc);
// 	gbc.gridx=1; gbc.gridy = 23;
// 	leftFramePanel.add(colorBarHideButton, gbc);
// 	gbc.gridx=0; gbc.gridy = 24;
// 	leftFramePanel.add(nodeNumsLabel, gbc);
// 	gbc.gridx=0; gbc.gridy = 25;
// 	leftFramePanel.add(nodeNumsShowButton, gbc);
// 	gbc.gridx=1; gbc.gridy = 25;
// 	leftFramePanel.add(nodeNumsHideButton, gbc);
// 	gbc.gridx=0; gbc.gridy = 26;
// 	leftFramePanel.add(elemNumsLabel, gbc);
// 	gbc.gridx=0; gbc.gridy = 27;
// 	leftFramePanel.add(elemNumsShowButton, gbc);
// 	gbc.gridx=1; gbc.gridy = 27;
// 	leftFramePanel.add(elemNumsHideButton, gbc); 
       

// 	leftPanel.add(addButton);
//         leftPanel.add(removeButton);
//         leftPanel.add(clearButton);




//Create a split pane with the two scroll panes in it.
			//splitPane = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT,
            //                           leftRootPanel, renderPanel);
            //splitPane.setOneTouchExpandable(true);
            //splitPane.setDividerLocation(150);

            //Provide minimum sizes for the two components in the split pane
	Dimension minimumSize = new Dimension(150, 250);
            //leftRootPanel.setMinimumSize(minimumSize);
          

//             splitPane2 = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT,
//                                        splitPane, treePanel);

//PAKLEIN
//	    splitPane2 = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT, leftRootPanel, treePanel);
// 	    splitPane2 = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT, leftRootScrollPane, treePanel);
//             splitPane2.setOneTouchExpandable(true);
//             splitPane2.setDividerLocation(400);

            //Provide minimum sizes for the two components in the split pane
            Dimension minimumSize2 = new Dimension(300, 250);
            treePanel.setMinimumSize(minimumSize);
	    //splitPane.setMinimumSize(minimumSize2);
            //renderPanel.setMinimumSize(minimumSize);
    //setLayout(new BorderLayout());
	    //treePanel.setPreferredSize(new Dimension(150,150));
	//add(treePanel);
// 	add(treePanel,new GridBagConstraints(1,2,1,1,0.05,1.0,GridBagConstraints.NORTHWEST,GridBagConstraints.BOTH,new Insets(0,0,0,0),0,0));
// 	add(renderPanel,new GridBagConstraints(2,2,1,1,0.8,1.0,GridBagConstraints.NORTHWEST,GridBagConstraints.BOTH,new Insets(0,0,0,0),0,0));
// 	add(leftRootPanel,new GridBagConstraints(0,2,1,1,0.05,1.0,GridBagConstraints.NORTHWEST,GridBagConstraints.BOTH,new Insets(0,0,0,0),0,0));
// 	add(new JSeparator(JSeparator.VERTICAL),new GridBagConstraints(1,2,1,3,0.0,0.0,GridBagConstraints.CENTER,GridBagConstraints.VERTICAL,new Insets(0,0,0,0),0,0));
	    //	add(splitPane,new GridBagConstraints(0,2,1,1,0.95,1.0,GridBagConstraints.NORTHWEST,GridBagConstraints.BOTH,new Insets(0,0,0,0),0,0));
	    //	add(splitPane2,new GridBagConstraints(0,2,1,1,1.0,.95,GridBagConstraints.NORTHWEST,GridBagConstraints.BOTH,new Insets(0,0,0,0),0,0));
	    // add(splitPane2, BorderLayout.SOUTH);
	    //add(leftRootScrollPane, BorderLayout.SOUTH);
  }
  
  public void actionPerformed(ActionEvent e){
    if (e.getActionCommand().equals("Exit")){
      System.exit(0);  
    }

    else if (e.getActionCommand().equals("Print value")){
      Print();
      //leftPanel.remove(addButton);
      //updateUI();
      
    }

    else if (e.getActionCommand().equals("Rotate")){
      String args;
      args = rotateXTF.getText() + " " + rotateYTF.getText() + " " + rotateZTF.getText();
      System.out.println("Rotate " + args);
      //DoCommand("Rotate", args);
    }

    else if (e.getActionCommand().equals("Pan")){
      String args;
      args = panXTF.getText() + " " + panYTF.getText();
      System.out.println("Pan " + args);
      //DoCommand("Pan", args);
    }

    else if (e.getActionCommand().equals("Zoom")){
      String args = zoomTF.getText();
      System.out.println("Zoom " + args);
      //DoCommand("Rotate", args);
    }
    
    else if (e.getActionCommand().equals("CommandLine")){
    CommandLine();
    }
    
    else if (e.getActionCommand().equals("BodyVarOK")){
      SetMinSc(Integer.parseInt(minScalarTF.getText()));
      
    }
    
    else if (e.getActionCommand().equals("ShowContours")){
      System.out.println("ShowContours");
    }
    
    else if (e.getActionCommand().equals("HideContours")){
      System.out.println("HideContours");
    }

    else if (e.getActionCommand().equals("ShowCutting")){
      System.out.println("ShowCuttingPlane");
       JDialog temp = new JDialog(parentFrame, true);
       temp.setTitle("Show Cutting Plane");
       temp.setSize(400, 300);
       JPanel cutPanel = new JPanel();
       JButton cutOKButton = new JButton("OK");
       JButton cutCancelButton = new JButton("Cancel");
       cutPanel.add(cutOKButton);
       cutPanel.add(cutCancelButton);
       temp.getContentPane().add(cutPanel);
   
       temp.pack();
       temp.show();
       
    }
    else if (e.getActionCommand().equals("HideCutting")){
      System.out.println("HideCuttingPlane");
    }
    else if (e.getActionCommand().equals("Update")){
		DoCommand("Update", "");
    }

    else if (e.getActionCommand().equals("FlipBook")){
      System.out.println("Flip Book");
    }
    else if (e.getActionCommand().equals("NextTimeStep")){
      System.out.println("Next Time Step");
    }
    else if (e.getActionCommand().equals("PreviousTimeStep")){
      System.out.println("Previous Time Step");
    }
    else if (e.getActionCommand().equals("SelectTimeStep")){
      System.out.println("Select Time Step");
    }
    else if (e.getActionCommand().equals("AddBody")){
	treePanel.addObject("New Node " + newNodeSuffix++);
	//AddScope("New Node " + newNodeSuffix++);
    }
    else if (e.getActionCommand().equals("RemoveBody")){
      treePanel.removeCurrentNode();
    }
    else if (e.getActionCommand().equals("Clear")){
       treePanel.clear();
    }
    else if (e.getActionCommand().equals("Interact")){
      Interact();
    }
    else if (e.getActionCommand().equals("Black BG")){
      System.out.println("Black BG");
    }
    else if (e.getActionCommand().equals("White BG")){
      System.out.println("White BG");
    }    
    else if (e.getActionCommand().equals("Surf")){
      DoCommand("Surface", "");
      DoCommand("Update", "");
    }
    else if (e.getActionCommand().equals("Wire")){
      DoCommand("Wire", "");
      DoCommand("Update", "");
    }
    else if (e.getActionCommand().equals("Points")){
      DoCommand("Point", "");
      DoCommand("Update", "");
    }
    else if (e.getActionCommand().equals("Show Axes")){
      DoCommand("ShowAxes", "");
    }
    else if (e.getActionCommand().equals("Hide Axes")){
      DoCommand("HideAxes", "");
    }
    else if (e.getActionCommand().equals("Show Color Bar")){
      System.out.println(e.getActionCommand());
    }
    else if (e.getActionCommand().equals("Hide Color Bar")){
      System.out.println(e.getActionCommand());
    }
    else if (e.getActionCommand().equals("Show Node Nums")){
      DoCommand("ShowNodeNum", "");
    }
    else if (e.getActionCommand().equals("Hide Node Nums")){
      DoCommand("HideNodeNum", "");
    }
    else if (e.getActionCommand().equals("Show Elem Nums")){
      DoCommand("ShowElemNum", "");
    }
    else if (e.getActionCommand().equals("Hide Elem Nums")){
      DoCommand("HideElemNum", "");
    }

  }

public void stateChanged(ChangeEvent e)
{
	if(e.getSource()==selectTimeSlider){
		selectTimeTF.setText(Integer.toString((int)selectTimeSlider.getValue()));
		String args;
		args = selectTimeTF.getText();
		//System.out.println(args);
		//DoCommand("SelectTimeStep", "args");
	}
	else if(e.getSource()==selectTimeSliderF){
		selectTimeTFF.setText(Integer.toString((int)selectTimeSliderF.getValue()));
	}

}
  
  
    public void populateTree(DynamicTree treePanel) {
        String f1Name = new String("0.0.frame");
	String b1Name = new String("0.body");
        String bd1Name = new String("0.body");

        DefaultMutableTreeNode p1, p2;

        p1 = treePanel.addObject(null, f1Name);
	p2 = treePanel.addObject(null, bd1Name);
        treePanel.addObject(p1, b1Name);

    }



} // class test
