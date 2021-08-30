# TCP/IP 멀티쓰레드를 활용한 빙고게임
## Index
  - [구현환경](#구현환경) 
  - [Getting Started](#getting-started)
  - [Contributing](#contributing)
  - [Authors](#authors)
  - [License](#license)
<!--  Other options to write Readme
  - [Deployment](#deployment)
  - [Used or Referenced Projects](Used-or-Referenced-Projects)
-->
## About Repository
<!--Wirte one paragraph of project description -->  
해당 레포짓토리는 멀티쓰레드 환경을 통해 다수의 클라이언트가 함께 게임을 즐길 수 있는 TCP/IP 멀티 프로그래밍 게임입니다. 

**원활한 게임 진행을 바라시면 구현 영상을 보시는 것을 추천드립니다.**

- 구현 영상 링크
- https://www.youtube.com/watch?v=q_6AyLAzF3w

## 구현환경
1. Linux & Window
2. ubuntu
3. C기반 코드

Bridged Network 설정을 통하여 가상 머신 컴퓨터에선 다른 IP주소를 할당함.
포트포워딩을 세팅하면 정상적으로 작동된다.

**설계환경**

<img width="528" alt="설계 환경" src="https://user-images.githubusercontent.com/54930877/123048618-8d132a80-d439-11eb-8888-c19103749036.PNG">

![image](https://user-images.githubusercontent.com/54930877/123050405-a2895400-d43b-11eb-8096-9c38db200f19.png)


## Getting Started
**레포짓토리 내 s.c 는 서버 코드, c.c 는 클라이언트 코드를 의미합니다.** 

**리눅스 환경에서 쓰레드 컴파일을 진행하시면 문제없이 실행 가능합니다.**

**포트포워딩을 하지 않으셨다면 ip주소는 루프백 주소를 사용하셔야합니다.**

**실행 전 소켓이 요구하는 인자값을 코드에서 확인 후 입력 바랍니다.** 

![image](https://user-images.githubusercontent.com/54930877/123049079-1d516f80-d43a-11eb-8d3b-a470b4e83f70.png)
왼쪽이 서버, 오른쪽이 클라이언트입니다. 
보시게 되면 IP주소가 다른 것으 확인 할 수가 있습니다. 이가 가능한 이유는 
윈도우 쪽으로 들어오는 접속을 VMWARE로 넘겨버리구요. 클라이언트가 윈도우로 접근할 시 
VMWARE로 연결되며 접속이 되는겁니다. 

**게임 내 접속**
![image](https://user-images.githubusercontent.com/54930877/123049198-440fa600-d43a-11eb-9772-e6a73c818980.png)
모든 클라이언트가 READY를 해야 게임이 시작됩니다.나머지 세개는 클라이언트입니다. 현재 READY를 해서 게임을 시작하기 전에 체팅을 하고 있는 모습입니다. 체팅을 치면 모든 클라이언트 화면에 출력되는 것을 볼 수 있습니다


**게임 내 READY & WAIT**
![image](https://user-images.githubusercontent.com/54930877/123049305-5ee21a80-d43a-11eb-957b-ed4b56bd375a.png)
클라이언트가 READT를 하면 서버쪽에 WAIT에서 READY로 바뀌는 것을 확인 가능하다.

![image](https://user-images.githubusercontent.com/54930877/123049388-76210800-d43a-11eb-8b76-bc842cb5d423.png)
지금은 게임을 하고 있는 중의 모습이다. turn 수가 표시되고, 빙고 수가 표시되고 있다. 빙고판은 25개의 숫자로 이루어져 있다. 빨간 색은 지워진 빙고를 의미한다. 서버쪽에선 게임과 관련된 진행로그가 표시된다.

**승&패**
![image](https://user-images.githubusercontent.com/54930877/123049509-981a8a80-d43a-11eb-82d5-8989a59a4bc2.png)
한 클라이언트에서 3개 빙고가 완성이 되었다.
그럼 이겼다라는 출력이 뜨고 그 외 클라이언트에겐 LOSE라는 내용이 출력되게 된다.


**무승부**
![image](https://user-images.githubusercontent.com/54930877/123049581-a9fc2d80-d43a-11eb-821e-19b0d07485f1.png)
한 클라이언트에서 22를 입력하게 되어서 3개 빙고가 동시에 만들어졌다. 그렇게 되면 
DRAW!! 라고 무승부라는 결과가 출력되게 된다.
게임 룰이 충족되어도 바로 게임이 종료되지 않는다.체팅이 가능하게 구현하여 축하 메시지도 보낼 수 있도록 구현을 하였다.


## Contributing
*동훈* - 메인 개발 및 WBS 구축

![image](https://user-images.githubusercontent.com/54930877/123050474-b6cd5100-d43b-11eb-9f55-12cd513b1fd0.png)

*동규*- 서브 개발

![image](https://user-images.githubusercontent.com/54930877/123050486-baf96e80-d43b-11eb-8d19-91f31d5e1ccf.png)


*경진* - 발표 영상 제작 및 ppt 

![image](https://user-images.githubusercontent.com/54930877/123050478-b92fab00-d43b-11eb-81b0-8e706b14ed82.png)


 *민* - 팀장, 발표, 문서화, 프로젝트 관리 
 
![image](https://user-images.githubusercontent.com/54930877/123050592-d5cbe300-d43b-11eb-8e2f-9b60a32bc1d2.png)



## Authors
  - **허민** - <huhmin0409@naver.com>
  - 프로젝트와 관련하여 궁금하시거나 필요하신 부분이 있으면 언제든지 메일 바랍니다.

## License

```
MIT License

Copyright (c) 2020 always0ne

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

