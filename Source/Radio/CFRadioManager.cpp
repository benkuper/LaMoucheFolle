/*
  ==============================================================================

	CFRadioManager.cpp
	Created: 19 Jun 2018 8:36:11am
	Author:  Ben

  ==============================================================================
*/


#include "CFRadioManager.h"
#include "Drone/DroneManager.h"

juce_ImplementSingleton(CFRadioManager);

CFRadioManager::CFRadioManager() :
	Thread("CFRadios"),
	shouldCheckRadios(true)
{
	startTimerHz(1);
	startThread();
}

CFRadioManager::~CFRadioManager()
{
	signalThreadShouldExit();
	waitForThreadToExit(2000);
}


void CFRadioManager::run()
{
	try
	{
		sleep(1000);
		LOG("Start radio thread");

		//stats
		//uint32 lastRadioCheckTime = 0;
		//uint32 lastPacketCheckTime = 0;
		int numPacketsSentSinceLastCheck = 0;
		int numAcksReceivedSinceLastCheck = 0;
		int numPacketsLostSinceLastCheck = 0;
		int maxLostCounter = 0;

		while (!threadShouldExit())
		{

			//uint32 curTime = Time::getApproximateMillisecondCounter();

			if (shouldCheckRadios)
			{
				setupRadios();
				shouldCheckRadios = false;
			}

			if (numRadios == 0)
			{
				sleep(100);
				continue;
			}

			OwnedArray<CFCommand> currentCommands;
			
			//Avoid risking race condition when creating the drone object
			if (DroneManager::getInstance()->items.size() == 0)
			{
				sleep(10);
				continue;
			}

			for (WeakReference<Drone> d : DroneManager::getInstance()->items)
			{
				//Check if object was deleted in the meantime (thread safe)
				if (d.wasObjectDeleted()) continue;
				if (!d->enabled->boolValue()) continue;


				//Lock the drone commmandQueue
				d->uniqueCommands.getLock().enter();
				currentCommands.addArray(d->uniqueCommands);
				d->uniqueCommands.clear();
				d->uniqueCommands.getLock().exit();
				
				d->commands.getLock().enter();
				for (int i = 0; i < d->commands.size(); i++)
				{
					if (d->commands[i] != nullptr) currentCommands.add(d->commands[i]);
				}
				d->commands.clear();
				d->commands.getLock().exit();


				//If drone is reachable has no safelink yet, send the safelink
				if (!d->safeLinkActive)
				{
					Drone::State ds = d->state->getValueDataAsEnum<Drone::State>();
					if (ds != Drone::DISCONNECTED) d->addCommand(CFCommand::createActivateSafeLink(d));
				}
				
				//If no command for this drone, add an empty packet, else, add all the commands
				//TODO : implement smart command depending on drone's state : if powered off, send only 1 or 2 ping/sec to not clog the radio / if drone is flying, send position instead of ping

				/*if (d->commandQueue.size() == 0)
				{
					CFCommand * c = d->getDefaultCommand();
					if (c != nullptr) currentCommands.add(c);
				} else
				{*/
				//}

				//clear the command queue from the drone

				//Release the lock
			}

			if (currentCommands.size() == 0) sleep(2);

			int curRadio = 0;
			for (auto &c : currentCommands)
			{
				//Todo, threaded radio calls to have non blocking calls each 800 pk /s

				if (curRadio >= radios.size())
				{
					DBG("Radio " << curRadio << " doesn't exist");
					continue;
				}
				Crazyradio * r = radios[curRadio];

				if (c == nullptr)
				{
					DBG("Command is null, skipping");
					continue;
				}

				if (c->drone.wasObjectDeleted())
				{
					DBG("Drone was deleted while processing commands.");
					continue;
				}

				if (c->type == CFCommand::ACTIVATE_SAFELINK && c->drone->safeLinkActive)
				{
					DBG("Drone safeLink already active, skipping");
					continue;
				}

				int droneId = c->drone->id->intValue();

				ITransport::Ack ack;

				Drone::State ds = c->drone->state->getValueDataAsEnum<Drone::State>();

				int lostCounter = 0;
				bool packetIsValidated = false;
				
				int maxTries = ds != Drone::DISCONNECTED ? 10 : 1; //only one try when pinging powered off drones

				while (!packetIsValidated && lostCounter < maxTries && !threadShouldExit())
				{
					try
					{
						uint8_t ch = droneId * 2;
						uint64_t ad = 0xE7E7E7E700 + String(droneId).getHexValue32();

						r->setChannel(ch);
						r->setAddress(ad); 
						
						//modify the packet for safe link
						if (c->drone->safeLinkActive)
						{
							c->data.set(0, c->data[0] & 0xF3 | (int)c->drone->safeLinkUpFlag << 3 | (int)c->drone->safeLinkDownFlag << 2);
							//DBG("Safe link encode : " << c->data[0]);
						}

						
						

						r->sendPacket(c->data.getRawDataPointer(), c->data.size(), ack);

					} catch (std::runtime_error &e)
					{
						LOGWARNING("Error sending packet, maybe radio is disconnected ? Error : " << e.what());
						setupRadios();
						break;
					} 

					numPacketsSentSinceLastCheck++;

					if (c->drone.wasObjectDeleted()) break;

					packetIsValidated = processAck(c, ack);
					if (packetIsValidated)
					{
						numAcksReceivedSinceLastCheck++;
					} else
					{
						lostCounter++;
						//DBG("Packet lost on " << c->drone->niceName << " / " << c->getTypeString());
						numPacketsLostSinceLastCheck++;
					}
				}

				maxLostCounter = jmax(lostCounter, maxLostCounter);
				lostCounter = 0;

				curRadio = numRadios == 0 ? 0 : (curRadio + 1) % numRadios;
			}

			currentCommands.clear();
			//if (curTime > lastPacketCheckTime + 1000) //check every second
			//{
			//	packetsPerSeconds = numPacketsSentSinceLastCheck;
			//	 
			//	//float ratio = numAcksReceivedSinceLastCheck * 1.0f / numPacketsSentSinceLastCheck;
			//	//if(packetsPerSeconds > 0) LOG("Stats : " << packetsPerSeconds << " packets/s, " << maxLostCounter << " max packets retries, ratio : " << ratio);
			//	
			//	numPacketsSentSinceLastCheck = 0;
			//	numAcksReceivedSinceLastCheck = 0;
			//	numPacketsLostSinceLastCheck = 0; 
			//	maxLostCounter = 0;

			//	lastPacketCheckTime = curTime;
			//}
			

			//for debug
			//sleep(100);
		}

	} catch (std::runtime_error &e)
	{
		LOGERROR("Error on radio  thread : " << e.what());
	} catch (std::exception &e)
	{
		LOGERROR("Exception on radio thread : " << e.what());
	}

	LOG("End radio thread");

}

void CFRadioManager::timerCallback()
{
	shouldCheckRadios = true;
}

//Threaded functions

void CFRadioManager::setupRadios()
{

	int newNumRadios = Crazyradio::numDevices();

	//LOG("Checking radios, found " << newNumRadios); 
	if (numRadios != newNumRadios)
	{
		LOG("Num radios has changed, clear and add all");
		radios.clear();
		numRadios = newNumRadios;
		for (int i = 0; i < numRadios; i++)
		{
			Crazyradio * r = new Crazyradio(i);
			try
			{
				r->setMode(3);
				DBG("Radio set to CHAD mode");
			} catch (std::runtime_error &e)
			{
				DBG("Radio not compatible with CHAD Mode, keeping normal mode : " << e.what());
			}

			radios.add(r);
		}

		LOG("Added " << numRadios << " radios");
		sleep(200);
	}
}

bool CFRadioManager::processAck(CFCommand * command, ITransport::Ack &ack)
{
	WeakReference<Drone> drone = command->drone;

	if (drone.wasObjectDeleted()) return false;
		
	if (!ack.ack)
	{
		drone->noAckReceived();
		return false;
	}
	
	
	

	CFPacket packet(drone, ack);

	if (packet.type == CFPacket::UNKNOWN)
	{
		DBG("---------  Unknown packet");

		String s = "[";
		for (int i = 0; i < (int)command->data.size(); i++) s += String::toHexString(command->data[i]) + " ";
		s += "]";
		DBG("Sent data : " << s);

		s = "[";
		for (int i = 0; i < (int)ack.size; i++) s += String::toHexString(ack.data[i]) + " ";
		s += "]";
		DBG("Received Packet : " + s);

	}

	if (drone->safeLinkActive)
	{
		drone->safeLinkUpFlag = !drone->safeLinkUpFlag;

		if ((ack.data[0] & 0x04) != (drone->safeLinkDownFlag << 2))
		{
			//drone->safeLinkDownFlag = !drone->safeLinkDownFlag;
			//LOGWARNING("Safe link check failed");
			return false;
		} else
		{
			//DBG("Safe link check passed !");
			drone->safeLinkDownFlag = !drone->safeLinkDownFlag;
		}
	} else
	{
		if (packet.hasSafeLink)
		{
			DBG("Safe link is now activated");
			drone->safeLinkDownFlag = false;
			drone->safeLinkUpFlag = false;
			drone->safeLinkActive = true;
		} else
		{
			if (command->type != CFCommand::REBOOT_FIRMWARE && command->type != CFCommand::REBOOT_INIT)
			{
				DBG("Safe link is enabled but packet has no safelink");
			}
			drone->safeLinkActive = false;
		}
	}

	drone->packetReceived(&packet);
	
	return true;
}