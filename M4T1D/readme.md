# Memory Match Game

## Compilation Instructions

```bash
mpicc -o memory_match memory_match.c
```

## Running Instructions with GUI

```bash
mpirun \
    -np 1 xterm -T "Server" -e ./memory_match : \
    -np 1 xterm -T "Player 1" -e ./memory_match : \
    -np 1 xterm -T "Player 2" -e ./memory_match
```

## Running Instructions with 3 Terminals

```bash
mpirun -np 1 --oversubscribe ./memory_match
```
