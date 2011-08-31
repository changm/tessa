function testSomething(queue:Object, link:int) {
	print(queue.link);
}

var x = new Object();
x.link = 5;
print(x.link);
testSomething(x, null);
