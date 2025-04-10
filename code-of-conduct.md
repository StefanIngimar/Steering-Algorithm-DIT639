# Refined Code of Conduct 

The following code of conduct was created based on a group discussion and later refined using a local LLM (Mistral:7b). Both the group discussion and LLM suggestions can be found below this section. 

## How do we plan to collaborate?

The collaboration will be based on in-person meetings and online communication supported by Discord and OneDrive. The group will have at least 1 in-person meeting per week, preferably after lectures, with the main goal of discussing current progress and planning the upcoming work. On top of the in-person meetings the group agreed for the ad-hoc online meetings and communication. 

While each group member will still be assigned a set of tasks to work on independently, we recognize the potential downsides of isolated work, such as duplicated efforts and limited knowledge sharing. To mitigate this, the group will adopt a light Scrum-inspired workflow, using a shared [Kanban board on Trello](https://trello.com/invite/b/67ec13bc25951d8ffd1a7992/ATTIb20e8406904ac5adb49602544946df211F579180/dit639
) to maintain visibility of current tasks, assignments, and progress. 

Daily stand-ups—either in person or via Discord—will be held to keep everyone informed and accountable, ensuring we track our collective velocity and progress. A weekly refinement session will be used to maintain a healthy backlog, clarify task scope, and prepare upcoming work in a way that promotes coordination and prevents overlapping responsibilities. 

For tasks that are larger in scope, have higher story points, or require more effort, the group may opt to use pair programming or swarming techniques. This approach will help tackle complex issues collaboratively, promote active knowledge sharing, and improve problem understanding across the team.

## How do we ensure that everyone in our group stays informed about the individual contributions?

As inferred above, the knowledge transfer among team members will be primarily communication-based. 

Weekly in-person meetings are a core opportunity to discuss solutions, provide feedback, and resolve misunderstandings. These will be supplemented by ad-hoc online communication when quick clarification is needed. 

To further strengthen knowledge sharing in a workflow where individuals take ownership of tasks, the group agreed on the following measures: 

1. Thorough code reviews will be conducted on all pull requests, with feedback and discussion encouraged before merging. 
2. Knowledge-sharing sessions will be held for complex or high-impact tasks, allowing for walkthroughs and group learning. 
3. Developers are also expected to take initiative in reading and understanding merged code on the main branch to stay aligned with system changes and design decisions. 

On top of that, code reviews and assignment of different tasks per team member throughout the project will ensure a steady increase in breadth of knowledge in the team. To further support this, the group agreed to try at least once assigning a more complex task to multiple team members, to evaluate how collaboration affects the workflow and shared understanding. 

## How do we ensure knowledge transfer among our team members?

As could be inferred from above, the knowledge transfer among team members will be primarily communication based.  

Based on empirical knowledge, in-person meetings are a great opportunity to discuss solutions, provide feedback, and clear up any misunderstandings. Therefore, weekly in-person meetings will be used as a base to ensure knowledge transfer among team members. The in-person meetings will be supported by online communication when a need for a fast discussion is needed. 

On top of that, code reviews and assignment of different tasks per team member throughout the project will ensure a steady increase in breadth of knowledge in the team. 

To ensure a better knowledge transfer the group agreed to try at least once to assign more than one person to a more complex task and see how such collaboration would affect the overall workflow and problem understanding in the team. 

## What is our usual communication plan?

As mentioned before, at least one in-person meeting preferably after lectures. Thus, a meeting on Tuesday. The group also agreed on ad-hoc online communication whenever needed through Discord.  

## How will we solve conflicts?

Conflicts in the team will be solved through respectful and structured discussions where all parties have a chance to present their perspectives. If the discussion does not lead to a resolution, the group will vote on the presented solutions. The team also agreed to follow the "disagree and commit" principle, as popularized by Jeff Bezos, to avoid prolonged arguments and keep progress moving.

To avoid conflicts in the first place, the team will:
- Clearly define task scopes and responsibilities
- Maintain open and respectful communication at all times
- Use Trello and daily stand-ups to avoid misunderstandings or duplicated work
- Encourage early discussion of blockers or concerns

In the rare case of an unresolved or serious conflict, the team will involve the course instructor for guidance and resolution support.

## How do you plan to ensure responsible use of LLMs in your project and how do you transparently and traceably document the use of LLMs?

The group has come to the consesus that we will not use LLMs in our project. Our observations have yielded that LLM response times are incredibly slow, and act as a bottleneck to our speedy progression; our efficiency is higher searching the web for relevant documentation to complete tasks. We have also realized that LLMs are prone to hallucinations when we cross-reference information on the internet with more trusted sources. At this point in time, we consider LLM support to be more of a liability rather than a helpful too.

**In the unlikely event we decide to change our minds again, we will revert back to our previous strategy with transparency of use of LLMs:**

LLMs will be used to help expand knowledge and improve created solutions and not produce solutions to the problems. That means that fully generated solutions by the LLMs are not acceptable and will not be merged into the main branch in the project.  

To ensure the proper use of LLMs as helpers and not solvers a person that is assigned to a task has to be able to fully explain their solution to other team members. A part of the in-person meetings will be used to talk about how and if LLMs have been used to solve the problem. 

If a task is solved with the help of an LLM, prompts must be included in the description of the merge request. Moreover, prompt screenshots must be added to a dedicated folder on OneDrive (where the folder name clearly indicates the issue/task that a person worked on). 

# Group Answers (No LLM) 

**How do we plan to collaborate?**

The collaboration will be based on in-person meetings and online communication supported by Discord and OneDrive. The group will have at least 1 in-person meeting per week, preferably after lectures, with the main goal of discussing current progress and planning the upcoming work. On top of the in-person meetings the group agreed for the ad-hoc online meetings and communication. 

The group agreed to work more independently on tasks. That means that each week every group member will be assigned a set of tasks over which they will have full control and will have to take full responsibility for their completion in a timely manner. 

**How do we ensure that everyone in our group stays informed about the individual contributions?** 

A part of the mentioned in-person meetings will be dedicated towards presenting solutions to the assigned tasks with the goal of creating a shared understanding about the task’s solution, creating a foundation for discussion, and chances to provide feedback. 

Moreover, the group agreed to utilize merge requests as a part of their git workflow where each merge request must be reviewed and approved by a team member that did not work on the task in question. 

**How do we ensure knowledge transfer among our team members?** 

As inferred above, the knowledge transfer among team members will be primarily communication based.  

Based on empirical knowledge, in-person meetings are a great opportunity to discuss solutions, provide feedback, and clear up any misunderstandings. Therefore, weekly in-person meetings will be used as a base to ensure knowledge transfer among team members. The in-person meetings will be supported by online communication when a need for a fast discussion is needed. 

On top of that, code reviews and assignment of different tasks per team member throughout the project will ensure a steady increase in breadth of knowledge in the team. 

**What is our usual communication plan?** 

As mentioned before, at least 1 in-person meeting preferably after lectures. Thus, a meeting on Tuesday. Ad-hoc online communication whenever needed through Discord.   

**How will we solve conflicts?** 

By means of a duel—honorably fought with rapiers and gunpowder weapons, where skill, strategy, and courage determine the victor. 

Conflicts in the team will be solved by means of a gentleman-like discussion where all parties will have a chance to talk and present their way of thinking. After the discussion the conflict will be resolved by all team-members voting on the solutions presented. 

The team members agreed to mitigate the conflict situations as much as possible and utilize the “disagree and commit” principle used by Jeff Bezos himself, to save time and reduce arguing. 

In a case of total moral devastation, the teacher will be involved to help resolve the conflict. 

**How do you plan to ensure responsible use of LLMs in your project and how do you transparently and traceably document the use of LLMs?** 

The group agreed that LLMs will be used to help expand knowledge and improve created solutions and not produce solutions to the problems. That means that fully generated solutions by the LLMs are not acceptable and will not be merged into the main branch in the project.  

To ensure the proper use of LLMs as helpers and not solvers a person that is assigned to a task has to be able to fully explain their solution to other team members.  

If a task is solved with the help of an LLM, prompts have to be included in the description of the merge request. 

# LLM Suggestions 

To increase the knowledge transfer between the team members, the LLM suggested considering pair-programming sessions or splitting tasks in a way that would make team members work on a specific task together. 

The pair-programming idea was not received positively in the group, but the concept of assigning more complex tasks to more than 1 team member sounded like a valid idea that the group could utilize (or at least try once) to see how it would affect the knowledge transfer and topic understanding in the group. 

The LLM also suggested incorporating a discussion into in-person meetings about if and how a LLM was used to create a solution for a task. That way a more responsible way of using LLMs would be shown in a group. Moreover, it was also suggested to maintain a log or history where LLMs were used throughout the project. The group agreed that screenshots of LLM conversations could be uploaded to dedicated OneDrive folders allowing for transparent use of LLMs and better traceability of when and how a model was used. 