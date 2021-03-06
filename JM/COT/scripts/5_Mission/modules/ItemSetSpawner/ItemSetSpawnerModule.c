class ItemSetSpawnerModule: EditorModule
{
	protected ref ItemSetSettings settings;
	protected ref ItemSetSpawnerMeta meta;

	void ItemSetSpawnerModule()
	{
		GetRPCManager().AddRPC( "COT_ItemSetSpawner", "LoadData", this, SingeplayerExecutionType.Client );
		GetRPCManager().AddRPC( "COT_ItemSetSpawner", "SpawnSelectedPlayers", this, SingeplayerExecutionType.Server );

		GetPermissionsManager().RegisterPermission( "ItemSets.View" );
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "ItemSets.View" );
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		if ( GetGame().IsClient() )
			GetRPCManager().SendRPC( "COT_ItemSetSpawner", "LoadData", new Param, true );
	}

	override void ReloadSettings()
	{
		super.ReloadSettings();

		settings = ItemSetSettings.Load();

		for ( int j = 0; j < settings.ItemSets.Count(); j++ )
		{
			string base = settings.ItemSets.GetKey( j );
			base.Replace( " ", "." );
			GetPermissionsManager().RegisterPermission( "ItemSets." + base );
		}
	}

	override void OnMissionFinish()
	{
		super.OnMissionFinish();

		if ( GetGame().IsServer() )
			settings.Save();
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/gui/layouts/ItemSets/ItemSetsMenu.layout";
	}

	ref array< string > GetItemSets()
	{
		return meta.ItemSets;
	}

	private void FillCar( Car car, CarFluid fluid )
	{
		float cap = car.GetFluidCapacity( fluid );
		car.Fill( fluid, cap );
	}
	
	void LoadData( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if( type == CallType.Server )
		{
			GetRPCManager().SendRPC( "COT_ItemSetSpawner", "LoadData", new Param1< string >( JsonFileLoader< ItemSetSpawnerMeta >.JsonMakeData( ItemSetSpawnerMeta.DeriveFromSettings( settings ) ) ), true );
		}

		if( type == CallType.Client )
		{
			Param1< string > data;
			if ( !ctx.Read( data ) ) return;

			JsonFileLoader< ItemSetSpawnerMeta >.JsonLoadData( data.param1, meta );

			for ( int j = 0; j < meta.ItemSets.Count(); j++ )
			{
				string base = meta.ItemSets[j];
				base.Replace( " ", "." );
				GetPermissionsManager().RegisterPermission( "ItemSets." + base );
			}
		}
	}

	private bool DetermineWillFit( EntityAI fittingCargo, string ClassName )
	{
		TIntArray values = new TIntArray;
		GetGame().ConfigGetIntArray( "CfgVehicles " + ClassName + " itemSize", values );

		if ( values.Count() != 2 ) return false;

		int iwidth = values[0];
		int iheight = values[1];

		CargoBase cargo = fittingCargo.GetInventory().GetCargo();

		int cwidth = cargo.GetWidth();
		int cheight = cargo.GetHeight();

		if ( iwidth > cwidth ) return false;
		if ( iheight > cheight ) return false;

		return true;
	}

	private EntityAI SpawnItem( PlayerBase player, string ClassName )
	{
		vector position = player.GetPosition();

		position[0] = position[0] + ( Math.RandomInt( 1, 50 ) - 25 ) / 10.0;
		position[1] = position[1] + 0.1;
		position[2] = position[2] + ( Math.RandomInt( 1, 50 ) - 25 ) / 10.0;

		return EntityAI.Cast( GetGame().CreateObject( ClassName, position, false, true, true ) );
	}

	private EntityAI SpawnItemInContainer( string container, PlayerBase player, EntityAI chest, string ClassName, float numStacks, float stackSize )
	{
		EntityAI item;
		ItemBase itemBs;

		for ( float i = 0; i < numStacks; i++ )
		{
			item = EntityAI.Cast( chest.GetInventory().CreateInInventory( ClassName ) );

			if ( item )
			{
				itemBs = ItemBase.Cast( item );

				itemBs.SetQuantity( stackSize );
			} else 
			{
				if ( DetermineWillFit( chest, ClassName ) && container != "" && chest )
				{
					chest = SpawnItem( player, container );

					item = EntityAI.Cast( chest.GetInventory().CreateInInventory( ClassName ) );

					itemBs = ItemBase.Cast( item );

					itemBs.SetQuantity( stackSize );
				} else
				{
					item = SpawnItem( player, ClassName );

					itemBs = ItemBase.Cast( item );

					itemBs.SetQuantity( stackSize );
				}
			}
		}

		return chest;
	}

	void SpawnSelectedPlayers( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		ref Param2< string, ref array< string > > data;
		if ( !ctx.Read( data ) ) return;

		array< ref AuthPlayer > players = DeserializePlayersID( data.param2 );

		ref ItemSetFile file = settings.ItemSets.Get( data.param1 );

		if ( file == NULL ) return;

		ref array< ref ItemSetItemInformation > parts = file.Items;

		if ( parts.Count() == 0 )
		{
			return;
		}

		string perm = file.Name;
		perm.Replace( " ", "." );

		if ( !GetPermissionsManager().HasPermission( "ItemSets.Spawn.ItemSets." + perm, sender ) )
		{
			return;
		}
		
		if( type == CallType.Server )
		{
			for ( int i = 0; i < players.Count(); i++ )
			{
				EntityAI chest;

				if ( file.ContainerClassName != "" )
				{
					chest = SpawnItem( players[i].PlayerObject, file.ContainerClassName );
				}

				for (int j = 0; j < parts.Count(); j++)
				{
					chest = SpawnItemInContainer( file.ContainerClassName, players[i].PlayerObject, chest, parts[j].Item, parts[j].NumberOfStacks, parts[j].StackSize );
				}

				COTLog( sender, "Loot chest " + data.param1 + " spawned on " + players[i].GetSteam64ID() );
			}

		}
	}
}