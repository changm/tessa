package FlogRayTracer {
public class Engine {
    public var options;
    
    
    public function Engine(options = null){
        if (options) {
          this.options = options;
        } else {
          this.options = {
                canvasHeight: 100,
                canvasWidth: 100
            };
        }

    }
} // class Engine

function renderScene(){
    var imageWidth = 100; // $F('imageWidth');
    var imageHeight = 100; // $F('imageHeight');

    var raytracer = new Engine(
        {
            canvasWidth: imageWidth,
            canvasHeight: imageHeight
        }
    );

    print(raytracer.options.canvasWidth);
}

renderScene();

} // package
