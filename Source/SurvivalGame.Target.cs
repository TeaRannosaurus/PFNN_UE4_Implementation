// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class SurvivalGameTarget : TargetRules
{
	public SurvivalGameTarget(TargetInfo Target) : base(Target)
	{
	    Type = TargetType.Game;
	    bUsesSteam = true;

	    if(Target.Configuration == UnrealTargetConfiguration.Shipping)
	    {
	        bUseLoggingInShipping = true;
	        bUsePDBFiles = true;
	    }

        ExtraModuleNames.Add("SurvivalGame");
    }
}
