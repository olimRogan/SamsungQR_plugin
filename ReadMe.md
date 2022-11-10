삼성 QR - 언리얼엔진 4.26 플러그인
---
- 필수
DefaultEngine.ini 에서
[HTTPServer.Listeners]
DefaultBindAddress=0.0.0.0 넣어줘야 외부IP 가능 (현재는 로컬 테스트만 했다.)
-> 프로젝트 패키징 할 때 설정해줘야 한다.

 - HttpServerModule 사용
 - Postman을 통해 테스트
 - 참고 사이트 (https://zhuanlan.zhihu.com/p/427465861 , https://qiita.com/peke2/items/41bf3357b064ac9564ef)
 
QR 이미지 교체
-> LoadQRTexture_FromFile 함수를 통해서 로컬에 있는 이미지를 가져와서 Texture2D 로 전달
-> Plugins/Samsung_QR/Content/QR_code.png 이미지를 교체해주면 된다. (경로와 파일이름 주의)

Http Post Method를 통해서 Data를 전달 받는다.
-> Json 데이터를 잘 전달 받는 것으로 확인.
-> param 의 Key, Value 모두 전달 받는다.
-> 성공적으로 전달 받으면 델리게이트를 통해서 엔진 내부에서 실행할 동작 바인드
-> 현재는 QR 위젯을 핸들링한다. (Visible, Hidden)

실행 로직
0. NativeConstruct 에서 실행 																									- 생성자
1. HttpServerModule 변수에 FHttpServerModule::Get() 을 통해서 할당
2. Http Server 생성 후 Port 지정
3. RouterHandle 에 BindRoute를 해준다. 여기서 Verb 와 Callback을 바인딩
4. 설정을 마친 후 Server->StartAllListeners() 함수를 통해서 서버 실행
5. NativeDestruct 에서 Server->StopAllListeners() 함수를 통해 서버 종료 및 Router->UnbindRoute(RouteHandle) 을 통해 언바인딩 시켜준다.	- 소멸자

