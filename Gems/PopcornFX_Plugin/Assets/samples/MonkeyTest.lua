-- This script test random actions on PopcornFX effects

local MonkeyTest =
{
	--exposed properties
	Properties =
	{
		-- The following effects are in the pack "Samples" that you can download online in the PopcornFX v2 editor:
		EffectsToSpawn = { "Particles/Blast.pkfx", "Particles/TorchFire.pkfx", "Particles/Stars.pkfx" },
	},
}

function MonkeyTest:OnActivate()
	self.EffectsSpawned = { };
	self.tickBusHandler = TickBus.Connect(self);
end

function MonkeyTest:GetRandomSceneEvent()
	local eventIdx = 0;
	local eventIsValid = false;
	repeat
		eventIdx = math.random(0, 2);
		if eventIdx == 0 then
			-- eventIdx == 0: Spawn emitter
			eventIsValid = table.getn(self.Properties.EffectsToSpawn) ~= 0;
		elseif eventIdx == 1 then
			-- eventIdx == 1: Destroy emitter
			eventIsValid = table.getn(self.EffectsSpawned) ~= 0;
		elseif eventIdx >= 2 then
			-- eventIdx == 2: Emitter action
			eventIsValid = table.getn(self.EffectsSpawned) ~= 0;
		end
	until eventIsValid
	return eventIdx;
end


function MonkeyTest:OnTick(deltaTime, timePoint)
	local 	eventCount = 15;
	repeat
		local eventIdx = self:GetRandomSceneEvent();
		if eventIdx == 0 then
			-- Spawn a new emitter:
			local randEffectIdx = math.random(1, table.getn(self.Properties.EffectsToSpawn));
			local effectPath = self.Properties.EffectsToSpawn[randEffectIdx];
			local transform = TransformBus.Event.GetWorldTM(self.entityId);
			local randPosInBox = transform:GetColumn(3) + Vector3(math.random(-2.0, 2.0), math.random(-2.0, 2.0), math.random(-2.0, 2.0));
			transform:SetColumn(3, randPosInBox);
			local emitterIdx = PopcornFXRequestBus.Broadcast.SpawnEmitterEntityAtLocation(effectPath, transform, true);
			table.insert(self.EffectsSpawned, emitterIdx);
			Debug.Log("Create emitter");
		elseif eventIdx == 1 then
			-- Destroy an emitter:
			local randEffectIdx = math.random(1, table.getn(self.EffectsSpawned));
			local emitterEntity = self.EffectsSpawned[randEffectIdx];
			table.remove(self.EffectsSpawned, randEffectIdx);
			GameEntityContextRequestBus.Broadcast.DestroyGameEntity(emitterEntity);
			Debug.Log("Destroy emitter");
		elseif eventIdx == 2 then
			local randEffectIdx = math.random(1, table.getn(self.EffectsSpawned));
			local emitter = self.EffectsSpawned[randEffectIdx];
			local emitterEventIdx = self:GetRandomEmitterEvent(emitter);
			self:DoEmitterEvent(emitter, emitterEventIdx);
		end
		eventCount = eventCount - 1;
	until eventCount == 0
end

function MonkeyTest:GetRandomEmitterEvent(emitter)
	local attribCount = PopcornFXEmitterComponentRequestBus.Event.GetAttributesCount(emitter);
	local attribSamplerCount = PopcornFXEmitterComponentRequestBus.Event.GetAttributeSamplersCount(emitter);
	local eventIdx = 0;
	local eventIsValid = false;
	repeat
		eventIdx = math.random(0, 5);
		if eventIdx == 0 then
			-- eventIdx == 0: SetTranform
			eventIsValid = true;
		elseif eventIdx == 1 then
			-- eventIdx == 1: Start
			eventIsValid = not PopcornFXEmitterComponentRequestBus.Event.IsPlaying(emitter);
		elseif eventIdx == 2 then
			-- eventIdx == 2: Stop
			eventIsValid = PopcornFXEmitterComponentRequestBus.Event.IsPlaying(emitter);
		elseif eventIdx == 3 then
			-- eventIdx == 3: Kill
			eventIsValid = PopcornFXEmitterComponentRequestBus.Event.IsPlaying(emitter);
		elseif eventIdx == 4 then
			-- eventIdx == 4: SetAttribute
			eventIsValid = attribCount ~= 0;
		elseif eventIdx == 5 then
			-- eventIdx == 5: SetSampler
			eventIsValid = false; -- attribSamplerCount ~= 0;
		end
	until eventIsValid
	return eventIdx;
end

function MonkeyTest:DoEmitterEvent(emitter, eventIdx)
	if eventIdx == 0 then
		-- eventIdx == 0: SetTranform
		Debug.Log("Set emitter transform");
		local transform = TransformBus.Event.GetWorldTM(self.entityId);
		local randPosInBox = transform:GetColumn(3) + Vector3(math.random(-2.0, 2.0), math.random(-2.0, 2.0), math.random(-2.0, 2.0));
		transform:SetColumn(3, randPosInBox);
		TransformBus.Event.SetWorldTM(emitter, transform);
	elseif eventIdx == 1 then
		-- eventIdx == 1: Start
		Debug.Log("Start emitter");
		PopcornFXEmitterComponentRequestBus.Event.Start(emitter);
	elseif eventIdx == 2 then
		-- eventIdx == 2: Stop
		Debug.Log("Stop emitter");
		PopcornFXEmitterComponentRequestBus.Event.Stop(emitter);
	elseif eventIdx == 3 then
		-- eventIdx == 3: Kill
		Debug.Log("Kill emitter");
		PopcornFXEmitterComponentRequestBus.Event.Kill(emitter);
	elseif eventIdx == 4 then
		-- eventIdx == 4: SetAttribute
		Debug.Log("Set emitter attribute");
		self:SetRandomAttribute(emitter);
	elseif eventIdx == 5 then
		-- eventIdx == 5: SetSampler
		Debug.Log("Set emitter attribute sampler");
	end
end

function MonkeyTest:SetRandomAttribute(emitter)
	local attribCount = PopcornFXEmitterComponentRequestBus.Event.GetAttributesCount(emitter);
	local attribIdx = math.random(0, attribCount - 1); -- The attributes idx starts at 0!!!
	local attribType = PopcornFXEmitterComponentRequestBus.Event.GetAttributeType(emitter, attribIdx);

	if attribType == 0 then
		Debug.Log("Set attribute of type bool");
		-- attribType == 1: Type_Bool
		local randX = math.random(0, 1) == 1;
		PopcornFXEmitterComponentRequestBus.Event.SetAttributeAsBool(emitter, attribIdx, randX);
	elseif attribType == 1 then
		Debug.Log("Set attribute of type bool2");
		-- attribType == 2: Type_Bool2
		local randX = math.random(0, 1) == 1;
		local randY = math.random(0, 1) == 1;
		PopcornFXEmitterComponentRequestBus.Event.SetAttributeAsBool(emitter, attribIdx, randX, randY);
	elseif attribType == 2 then
		Debug.Log("Set attribute of type bool3");
		-- attribType == 2: Type_Bool3
		local randX = math.random(0, 1) == 1;
		local randY = math.random(0, 1) == 1;
		local randZ = math.random(0, 1) == 1;
		PopcornFXEmitterComponentRequestBus.Event.SetAttributeAsBool(emitter, attribIdx, randX, randY, randZ);
	elseif attribType == 3 then
		Debug.Log("Set attribute of type bool4");
		-- attribType == 3: Type_Bool4
		local randX = math.random(0, 1) == 1;
		local randY = math.random(0, 1) == 1;
		local randZ = math.random(0, 1) == 1;
		local randW = math.random(0, 1) == 1;
		PopcornFXEmitterComponentRequestBus.Event.SetAttributeAsBool(emitter, attribIdx, randX, randY, randZ, randW);
	elseif attribType == 4 then
		Debug.Log("Set attribute of type float");
		-- attribType == 4: Type_Float
		local randX = math.random() + math.random(-1000, 1000);
		PopcornFXEmitterComponentRequestBus.Event.SetAttributeAsFloat(emitter, attribIdx, randX);
	elseif attribType == 5 then
		Debug.Log("Set attribute of type float2");
		-- attribType == 5: Type_Float2
		local randX = math.random() + math.random(-1000, 1000);
		local randY = math.random() + math.random(-1000, 1000);
		PopcornFXEmitterComponentRequestBus.Event.SetAttributeAsFloat(emitter, attribIdx, randX, randY);
	elseif attribType == 6 then
		Debug.Log("Set attribute of type float3");
		-- attribType == 6: Type_Float3
		local randX = math.random() + math.random(-1000, 1000);
		local randY = math.random() + math.random(-1000, 1000);
		local randZ = math.random() + math.random(-1000, 1000);
		PopcornFXEmitterComponentRequestBus.Event.SetAttributeAsFloat(emitter, attribIdx, randX, randY, randZ);
	elseif attribType == 7 then
		Debug.Log("Set attribute of type float4");
		-- attribType == 7: Type_Float4
		local randX = math.random() + math.random(-1000, 1000);
		local randY = math.random() + math.random(-1000, 1000);
		local randZ = math.random() + math.random(-1000, 1000);
		local randW = math.random() + math.random(-1000, 1000);
		PopcornFXEmitterComponentRequestBus.Event.SetAttributeAsFloat(emitter, attribIdx, randX, randY, randZ, randW);
	elseif attribType == 8 then
		Debug.Log("Set attribute of type int");
		-- attribType == 8: Type_Int
		local randX = math.random(-1000, 1000);
		PopcornFXEmitterComponentRequestBus.Event.SetAttributeAsInt(emitter, attribIdx, randX);
	elseif attribType == 9 then
		Debug.Log("Set attribute of type int2");
		-- attribType == 9: Type_Int2
		local randX = math.random(-1000, 1000);
		local randY = math.random(-1000, 1000);
		PopcornFXEmitterComponentRequestBus.Event.SetAttributeAsInt(emitter, attribIdx, randX, randY);
	elseif attribType == 10 then
		Debug.Log("Set attribute of type int3");
		-- attribType == 10: Type_Int3
		local randX = math.random(-1000, 1000);
		local randY = math.random(-1000, 1000);
		local randZ = math.random(-1000, 1000);
		PopcornFXEmitterComponentRequestBus.Event.SetAttributeAsInt(emitter, attribIdx, randX, randY, randZ);
	elseif attribType == 11 then
		Debug.Log("Set attribute of type int4");
		-- attribType == 11: Type_Int4
		local randX = math.random(-1000, 1000);
		local randY = math.random(-1000, 1000);
		local randZ = math.random(-1000, 1000);
		local randW = math.random(-1000, 1000);
		PopcornFXEmitterComponentRequestBus.Event.SetAttributeAsInt(emitter, attribIdx, randX, randY, randZ, randW);
	elseif attribType == 12 then
		Debug.Log("Set attribute of type quaternion");
		local axisOfRotation = NormalizeVector(Vector3(math.random(), math.random(), math.random()));
		local randQuaternion = Quaternion.CreateFromAxisAngle(axisOfRotation, math.random() * math.pi * 2.0):GetNormalized()
		PopcornFXEmitterComponentRequestBus.Event.SetAttributeAsQuaternion(randQuaternion);
		-- attribType == 12: Type_Quaternion
	else
		Debug.Log("Cannot set attribute of unknown type");
		-- attribType == -1: Type_Unknown
	end
end

function MonkeyTest:OnDeactivate()
	if (self.tickBusHandler) then
		self.tickBusHandler:Disconnect();
	end
end

return MonkeyTest
