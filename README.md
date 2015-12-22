# Dumb BMS State Machine
This is a simple Arduino program that controls whether or not an electric vehicle should allow discharging based on three inputs and a state machine. This program is set up to use the Velleman KA05 I/O shield as a vehicle interface.

## I/O
There are two outputs connected to 1A relays:
* Main contactor (allows discharging)
  * First relay from the left, pin 8 in Arduino land
* Charger enable (starts/stops charger)
  * Second relay, pin 9

Three active low inputs:
* BMS OK/not OK signal
  * labeled as digital input 6, Arduino pin 7
* Key switch
  * DIO 5, Arduino pin 6
* AC power connected
  * DIO 4, Arduino pin 5

## State Machine
Four states:
* STANDBY:
  * Default state when the key is off and the battery is ready.
* DISCHARGING:
  * They key is on and the battery is ready. The main contactor will be engaged.
* EMPTY:
  * The battery is not ready, and the contactor is disengaged regardless of key position.
* CHARGING:
  * AC power is connected and the BMS has not yet indicated the battery is full. The charger is enabled.
