# UmikoBot TODO

### Bug Fixes

- Wallet should show the avatar of the person
- The levels module (`levels.json`) stores data differently to the other modules
	- Most other modules go
	```
	server1 {
		data1: ...,
    	data2: ...,
	},
	server2 {
    	data1: ...,
    	data2: ...,
	}
	```
	- `levels.json` does
	```
	data1 {
		server1: ...,
    	server2: ...,
	},
	data2 {
    	server1: ...,
    	server2: ...,
	}
	```

### Features

- Help description should be better
	- Should be sorted (either alphabetically or by module)
	- `!help module` should exist (to show commands specific to that module)
	- Instead of showing "**Wrong usage of command**", show the help message?

- Richlist/wallet:
	- Should it show your ranking (like the XP system does)?
	- This could be added to `!wallet` or `!status` as well

- Things to buy:
	- Bail: should be able to get out of jail for a price (maybe 150 - 200 coins)
		- This shouldn't conflict with the current jail system as the thief already has to pay a percentage of the amount they tried to steal
		- E.g. if they tried to steal 10000 coins and failed, they'd have to pay 5000 coins then either wait the jail time or pay extra to get out
	- Sponsor: reduces the price of perks for *someone else* for a limited time
	- Memes and jokes: sourced from reddit
		- They should cost a small amount (maybe 10 coins) to incentivise people buying it
	- Potential high-risk/high-reward mode for `!steal`
		- It could decrease steal success chance but award a bonus amount (e.g. 25%) for a successful steal
		- A low-risk/low-reward mode could do the opposite?

- Perks:
	- Maybe you can sell them back to the bot for coins?
		- Maybe to other people too (have them negotiate the price)
		- The bot should buy them back at a lower price than what it sold the perks for

- Steal:
	- The more coins you're trying to steal, the riskier it should be

- Events:
	- Some sort of event system to earn coins by doing things?
	- Or events change the chances for steal / the fines, etc.
