#include <string>
#include <unordered_set>
#include <algorithm>
#include <cctype>

bool isCocoObject(const std::string& input) {
    // List of all 80 COCO object classes (lowercased) :contentReference[oaicite:0]{index=0}
    static const std::unordered_set<std::string> cocoClasses = {
        "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck",
        "boat", "traffic light", "fire hydrant", "stop sign", "parking meter", "bench",
        "bird", "cat", "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra",
        "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
        "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove",
        "skateboard", "surfboard", "tennis racket", "bottle", "wine glass", "cup",
        "fork", "knife", "spoon", "bowl", "banana", "apple", "sandwich", "orange",
        "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
        "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse",
        "remote", "keyboard", "cell phone", "microwave", "oven", "toaster", "sink",
        "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
        "hair drier", "toothbrush"
    };

    // Normalize to lowercase for case-insensitive matching
    std::string s = input;
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    return cocoClasses.find(s) != cocoClasses.end();
}

// // Example usage
// #include <iostream>
// int main() {
//     for (auto test : {"Dog", "dragon", "Pizza", "TV", "rocket"}) {
//         std::cout << test << " → "
//                   << (isCocoObject(test) ? "YES" : "no")
//                   << "\n";
//     }
//     return 0;
// }

