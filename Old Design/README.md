# Storm Deck Calculator (prototype)

This is a small prototype in C that models a simplified Magic: The Gathering hand and searches (BFS) for a fastest route to win within the first 3 turns.

Build (Windows with MinGW or Linux):

```powershell
# from repository root
gcc -std=c11 -O2 -Wall -o storm main.c game.c deck.c cards_repo.c
# or if you have make available:
make
```

Run:

```powershell
./storm
```

Design notes:
- Simplified model: two card types: Mountain (land) and Lightning Bolt (deal 3 damage).
- Lands give +1 permanent mana and provide that mana immediately for this simplified model.
- Spells cost mana and have abilities implemented as C functions.
- BFS explores plays (play cards) and end-turn actions up to 3 turns and reports a shortest winning sequence if found.

Extending:
- Add more card types in `cards_repo.c` and implement their abilities.
- Replace the simple visited list with a hash set for larger searches.
- Improve rules (land play limits, tapping, mana colors, multiple players).
