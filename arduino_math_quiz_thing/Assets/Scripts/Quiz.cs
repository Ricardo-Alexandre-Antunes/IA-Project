using TMPro;
using UnityEngine;
using UnityEngine.InputSystem;

using UnityEditor;

public class Quiz : MonoBehaviour
{
    private TMP_Text[] text;
    private TMP_Text question;
    private TMP_Text[] options;
    private int curQuestion = 0;
    private QuizData quizData = new QuizData();
    private int quizSize;

    [SerializeField] private InputActionReference[] inputActions;

    // Start is called once before the first execution of Update after the MonoBehaviour is created
    void Start()
    {
        text = GetComponentsInChildren<TMP_Text>();
        Debug.Log(text);
        question = text[0];
        options = text[1..];
        quizData.LoadData();
        quizSize = quizData.GetLength();
        LoadQuestion(curQuestion);
    }

    // Update is called once per frame
    void Update()
    {
        foreach (var action in inputActions)
        {
            if (action.action.WasPressedThisFrame())
            {
                if (quizData.IsCorrect(curQuestion, ArrayUtility.IndexOf(inputActions, action) + 1))
                {
                    curQuestion++;
                    if (curQuestion < quizSize)
                    {
                        LoadQuestion(curQuestion);
                    }
                    else
                    {
                        // insert end of game here
                    }
                }
                else
                {
                    // wrong
                }
            }
        }
    }

    private void LoadQuestion(int curQuestion)
    {
        question.text = quizData.GetQuestion(curQuestion);
        for (int i = 0; i < options.Length; i++)
        {
            options[i].text = quizData.GetOption(curQuestion, i);
        }
    }
}
