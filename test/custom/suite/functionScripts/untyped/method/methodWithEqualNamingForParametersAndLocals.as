function testSomething(queue, link) {
	print(queue.link);
}

var x = new Object();
x.link = 5;
print(x.link);
testSomething(x, null);
