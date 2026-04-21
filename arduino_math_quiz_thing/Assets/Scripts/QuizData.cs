using UnityEngine;

public class QuizData
{
    private string[] questions = new string[2];
    private string[,] options = new string[2, 4];
    private int[] answers = new int[2];

    public void LoadData()
    {
        questions[0] = "1+1";
        options[0, 0] = "1";
        options[0, 1] = "2";
        options[0, 2] = "3";
        options[0, 3] = "11";
        answers[0] = 2;

        questions[1] = "8*9";
        options[1, 0] = "54";
        options[1, 1] = "56";
        options[1, 2] = "62";
        options[1, 3] = "72";
        answers[1] = 4;
    }
    
    public string GetQuestion(int curQuestion)
    {
        return questions[curQuestion];
    }

    public string GetOption(int curQuestion, int option)
    {
        return options[curQuestion, option];
    }

    public int GetLength()
    {
        return questions.Length;
    }

    public bool IsCorrect(int curQuestion, int option)
    {
        return answers[curQuestion] == option;
    }
}
