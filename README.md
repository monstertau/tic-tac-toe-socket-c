# tic-tac-toe-socket-c

## 1. How to run

### a. Server folder

run `make all` to build server
run `./bin/server` to run the server

### b. Client folder

run `make all` to build client
run `./game` to run the server

## 2. Message Design for Socket
- Message sẽ được định nghĩa theo định dạng `{COMMAND}~{DATA1}~{DATA2}....` ngăn cách bởi dấu `~` và luôn có command ở đầu mỗi lệnh
### a. Client
- Hiện tại, client sẽ có 2 command chính gửi lên cho server:
  - create: gửi lệnh tạo phòng kèm theo tên và size board (ví dụ `create~dung~3`)
  - join: gửi lệnh join phòng kèm theo tên và mã room phòng (ví dụ `join~dung~123456`)
- Khi đã có 2 socket kết nối tới room, mỗi khi đến lượt mình, client sẽ gửi mã `x~y` tương ứng với tọa độ x,y trên bàn
- TO BE UPDATED....
### b. Server
- Hiện tại, server mới có 3 command chính gửi về cho server:
  - status : gửi status thành công(1) hay lỗi(0) kèm theo message (ví dụ `status~1~create room successfully`)
  - update: gửi update board về cho client, đi kèm là label của bạn, label của đối thủ, size board và board game, vị trí trống là `-` (ví dụ `update~X~O~3--------X`)
  - moving: gửi command yêu cầu client nhập vị trí để đặt label vào board
- TO BE UPDATED....
## 3. Game Flow
- Server sẽ bao gồm struct GameManager để quản lý RoomList, với từng index ở trong RoomList sẽ là 1 struct GameBoard để quản lý từng game
```
typedef struct GameManager_ {
    GameBoard *RoomGameList[MAX_ROOM];
    pthread_mutex_t managerMutex;
} GameManager;
struct GameBoard_ {
    int roomID;
    pthread_cond_t gameCond;
    pthread_mutex_t gameMutex;
    int size;
    char board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
    Player *playerList[MAX_PLAYER];
    char winner;
};
struct Player_ {
    int sockfd;
    char name[30];
    bool isTurned;
    char label;
};
```
- đầu tiên, server sẽ lắng nghe connect từ bất kì 1 client socket nào. Khi 1 socket mới đến, server sẽ tạo ra 1 thread mới handle lệnh đầu tiên mà client gửi về (create,join,.....) ứng với menu option của client.Mỗi thread mới sẽ gọi hàm `void *roomManagement` handle từng loại command đã được nêu cho từng client tương ứng, xong rồi xóa thread. Chi tiết cách handling các loại command:
  - Với create, server sẽ check xem còn phòng trống ở trong list của GameManager. 
    - Nếu có thì trả về status 1 và tạo `GameBoard` cho index trong list tương ứng, tạo thread mới cho client đã tạo phòng gọi hàm `void *handleGameBoard` và chờ 1 client mới join tới. 
    - Nếu không thì trả về status 0 và gửi message lỗi
  - Với join, server sẽ kiểm tra nếu roomcode trùng với bất kì code nào trong list
    - Nếu không thì trả về status 0 và gửi message lỗi
    - Nếu có thì add player 2 vào trong gameboard và signal cho thread của `void *handleGameBoard` là player 2 đã join và bắt đầu trò chơi
- hàm `void *handleGameBoard` sẽ là handler chính để handling logic game. Đầu tiên, server sẽ gửi cho 2 player command `update` để show board tới từng client. Sau đó, server sẽ check sẽ tới turn ai, rồi gửi command `moving` cho người đó để yêu cầu di chuyển. Sau khi nhận được yêu cầu di chuyển, lặp lại bước gửi command `update` cho tới khi logic game nhận ra game đã kết thúc, lúc đó thông báo người thắng và close socket
