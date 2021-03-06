class UIActionBase extends ScriptedWidgetEventHandler 
{
	protected ref Widget layoutRoot;

	protected ref Widget m_Disable;

	protected Class m_Instance;
	protected string m_FuncName;

	protected bool m_HasCallback;

	protected ref UIActionData m_Data;

	void OnWidgetScriptInit( Widget w )
	{
		layoutRoot = w;
		layoutRoot.SetHandler( this );

		OnInit();

		m_Disable = layoutRoot.FindAnyWidget( "action_wrapper_disable" );
	}

	void OnInit() 
	{
	}

	void Show()
	{
		layoutRoot.Show( true );
		OnShow();
	}

	void Hide()
	{
		OnHide();
		layoutRoot.Show( false );
	}

	void OnShow()
	{
	}

	void OnHide() 
	{
	}

	void Disable()
	{
		layoutRoot.Enable( false );

		m_Disable.SetFlags( WidgetFlags.IGNOREPOINTER );
		m_Disable.Show( true );
	}

	void Enable()
	{
		layoutRoot.Enable( true );

		m_Disable.ClearFlags( WidgetFlags.IGNOREPOINTER );
		m_Disable.Show( false );
	}

	ref Widget GetLayoutRoot() 
	{
		return layoutRoot;
	}

	void SetCallback( Class instance, string funcname )
	{
		if ( instance == NULL || funcname == "" ) return;

		m_Instance = instance;
		m_FuncName = funcname;

		m_HasCallback = true;
	}

	bool CallEvent( UIEvent eid )
	{
		return false;
	}

	void SetData( ref UIActionData data )
	{
		m_Data = data;
	}

	ref UIActionData GetData()
	{
		return m_Data;
	}
}