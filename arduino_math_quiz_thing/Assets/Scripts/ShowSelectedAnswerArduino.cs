using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO.Ports;
using TMPro;

public class ShowSelectedAnswerArduino : MonoBehaviour
{
    SerialPort sp = new SerialPort("COM13", 9600);
    [SerializeField] private TMP_Text selectedAnswer;

    // Start is called before the first frame update
    void Start()
    {
        sp.Open();
        sp.ReadTimeout = 100;
    }

    // Update is called once per frame
    void Update()
    {
        if (sp.IsOpen)
        {
            try
            {
                if (sp.ReadByte() != 0)
                {
                    string answer = sp.ReadLine();
                    print(answer);
                    selectedAnswer.text = answer;
                }
            }
            catch (System.Exception)
            {

            }

        }
    }
}