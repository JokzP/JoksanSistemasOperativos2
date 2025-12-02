# TESTING.md - Test Plan and Results

## Basic Communication

1. Start server:
   ```bash
   ./chat_server
   ```
2. Start two clients:
   ```bash
   ./chat_client   # user: alice
   ./chat_client   # user: bob
   ```
3. Send broadcast messages and private messages:
   - `alice> Hello everyone!`
   - `bob> /msg alice Hi alice!`

Expected:
- Both see broadcast messages.
- Only alice sees the private message from bob.

## Multiple Clients

Use a loop to start multiple clients (manual username entry or scripted):

```bash
for i in $(seq 1 5); do
    ./chat_client &
done
```

Observe that the server lists all users with `/list`.

## Disconnections

- Have a client send `/quit`.
- Or kill a client process with `kill -9`.

The server should:
- Detect closed socket (`recv` returns 0).
- Remove the client from the linked list.
- Decrease `active_users`.

## Server Commands

From server stdin:

- `/list` – prints users.
- `/stats` – shows total messages, active users.
- `/broadcast Maintenance message` – pushes a system message to all clients.
- `/shutdown` – closes all connections and exits.

## Stress Test (Placeholder)

`tests/test_stress.sh` provides a skeleton to spawn many clients and send messages. It can be extended as needed.

## Error Handling

- Server handles `EINTR` from `select()` and continues.
- On any socket error in client/server, the corresponding connection is closed and cleaned up.
